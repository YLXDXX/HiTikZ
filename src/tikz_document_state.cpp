#include "tikz_document_state.h"
#include <QTextBlock>
#include <QTextDocument>

TikzDocumentState::TikzDocumentState()
{
    m_beginRe = QRegularExpression(QStringLiteral("\\\\begin\\s*\\{([^}]*)\\}"));
    m_endRe = QRegularExpression(QStringLiteral("\\\\end\\s*\\{([^}]*)\\}"));
    m_uselibRe = QRegularExpression(QStringLiteral("\\\\usetikzlibrary\\s*\\{([^}]*)\\}"));
    m_tikzsetRe = QRegularExpression(QStringLiteral("\\\\tikzset\\s*\\{"));
    m_tikzstyleRe = QRegularExpression(QStringLiteral("\\\\tikzstyle\\s*\\{([^}]*)\\}"));
    m_definecolorRe = QRegularExpression(QStringLiteral("\\\\definecolor\\s*\\{([^}]*)\\}"));
    m_colorletRe = QRegularExpression(QStringLiteral("\\\\colorlet\\s*\\{([^}]*)\\}"));
    // A TikZ option list [ ... ] may itself contain ']' inside a brace group
    // (e.g. label={[blue]right:$B$}). A naive "[^\]]*" stops at the first ']'
    // and swallows the wrong span, so the following (name) is never seen. This
    // sub-pattern matches a balanced option group, allowing brackets nested in
    // up to two levels of braces (covers essentially all real TikZ code).
    static const QString optGroup = QStringLiteral(
        "\\[(?:[^\\[\\]{}]|\\{(?:[^{}]|\\{[^{}]*\\})*\\})*\\]");
    m_coordinateRe = QRegularExpression(
        QStringLiteral("\\\\coordinate\\s*(?:") + optGroup +
        QStringLiteral("\\s*)?\\(([^)]+)\\)"));
    // The path-operation form 'coordinate (name)' (no backslash) appears inside
    // a \draw/\path/\fill path, e.g.
    //   \draw (2,1) coordinate (test) circle [radius=2mm];
    // It may carry an optional [options] group before the (name). A negative
    // look-behind on a letter/backslash keeps this from matching \coordinate or
    // words ending in "coordinate".
    m_coordOpRe = QRegularExpression(
        QStringLiteral("(?<![\\\\a-zA-Z])coordinate\\s*(?:") + optGroup +
        QStringLiteral("\\s*)?\\(([^)]+)\\)"));
    m_foreachRe = QRegularExpression(
        QStringLiteral("\\\\foreach\\s+((?:\\\\[a-zA-Z]+\\s*(?:/\\s*)?)+)"
                       "(?:\\s*") + optGroup + QStringLiteral(")?\\s*in"));
    m_newcmdRe = QRegularExpression(
        QStringLiteral("\\\\(?:new|renew|provide)command\\*?\\s*\\{?\\\\([a-zA-Z0-9@]+)"));
    m_defRe = QRegularExpression(
        QStringLiteral("\\\\(?:def|edef|gdef|xdef)\\s*\\\\([a-zA-Z0-9]+)"));
    m_letRe = QRegularExpression(
        QStringLiteral("\\\\let\\s*\\\\([a-zA-Z0-9@]+)"));
    // 'at (coord)' clause of a node/pic. The coordinate may contain one level
    // of nested parens, e.g. at ($(a)+(b)$) with the calc library.
    static const QString atClause = QStringLiteral(
        "at\\s*\\((?:[^()]|\\([^()]*\\))*\\)");
    // \node/\pic/\matrix command form. Verified against tikz.code.tex: after
    // the operation keyword, [options], (name) and at (coord) may appear in
    // ANY order and multiple times ("node[draw] (a) [rotate=10] {text}"), and
    // \matrix is literally \path node[matrix]. The (name) is captured wherever
    // it occurs in that sequence.
    m_nodeRe = QRegularExpression(
        QStringLiteral("\\\\(?:node|pic|matrix)(?![a-zA-Z])\\s*(?:(?:") + optGroup +
        QStringLiteral("|") + atClause +
        QStringLiteral(")\\s*)*\\(([^)]+)\\)"));
    // Path-operation form of node/pic (no backslash), e.g.
    //   \draw (0,0) ... node [op amp] (OA) {OA1};
    //   \draw (0,0) pic[red] (arc1) {angle=A--B--C};
    // The negative look-behind keeps this from re-matching \node/\pic and from
    // matching words ending in "node"/"pic". 'at (...)' groups are skipped so
    // "node at (1,0) {}" never registers the coordinate as a name.
    m_nodeOpRe = QRegularExpression(
        QStringLiteral("(?<![\\\\a-zA-Z@])(?:node|pic)(?![a-zA-Z])\\s*(?:(?:") + optGroup +
        QStringLiteral("|") + atClause +
        QStringLiteral(")\\s*)*\\(([^)]+)\\)"));
    m_styleInTikzsetRe = QRegularExpression(
        QStringLiteral("([\\w\\s-]+)/\\.(style|code|pic|append style|prefix style)"
                        "\\s*=\\s*\\{"));
    m_commentRe = QRegularExpression(QStringLiteral("%"));
    m_usepackageRe = QRegularExpression(
        QStringLiteral("\\\\usepackage\\s*(?:\\[[^\\]]*\\]\\s*)?\\{([^}]*)\\}"));
    // 'name path[ global| local]=<name>' assigns a name to the current path.
    // The value may be brace-wrapped or a bare token (allowing spaces and '--',
    // e.g. name path=D--F or name path={circle K}).
    m_namePathRe = QRegularExpression(
        QStringLiteral("name\\s+path(?:\\s+global|\\s+local)?\\s*=\\s*"
                       "(?:\\{([^}]*)\\}|([^,\\]\\}]+))"));
    // 'by={...}' inside \path[name intersections={...}] names the intersection
    // coordinates. Captures the full brace group for further comma parsing.
    m_byRe = QRegularExpression(QStringLiteral("\\bby\\s*=\\s*\\{([^{}]*(?:\\{[^{}]*\\}[^{}]*)*)\\}"));
    m_tikzEnvSet = {
        "tikzpicture","scope","pgfonlayer",
        "axis","semilogxaxis","semilogyaxis","loglogaxis",
        "polaraxis","smithchart","ternaryaxis","groupplot",
        "tikzcd","circuitikz","forest","mindmap","feynman"
    };
}

TikzDocumentState::~TikzDocumentState() = default;

void TikzDocumentState::addPackageImplications(const QString &pkg, QSet<QString> &libs)
{
    QString p = pkg.trimmed().toLower();
    // Tolerate an inline option group, e.g. "circuitikz[europeanvoltages]".
    int bracket = p.indexOf(QLatin1Char('['));
    if (bracket >= 0)
        p = p.left(bracket).trimmed();
    if (p.isEmpty())
        return;

    struct Mapping { const char *pkg; const char *lib; };
    static const Mapping mappings[] = {
        {"circuitikz",   "circuitikz"},
        {"tikz-3dplot",  "3d"},
        {"tikz-cd",      "cd"},
        {"tkz-euclide",  "tkz-euclide"},
        {"physics",      "physics"},
        {"siunitx",      "siunitx"},
        {"pgfplots",     "pgfplots"},
        {"chemfig",      "chemfig"},
        {"tikz-feynman", "tikz-feynman"},
    };
    for (const auto &m : mappings) {
        if (p == QLatin1String(m.pkg))
            libs.insert(QLatin1String(m.lib));
    }
}

void TikzDocumentState::clear()
{
    m_scopeStack.clear();
    m_activeLibs.clear();
    m_userStyles.clear();
    m_userCoords.clear();
    m_userNodes.clear();
    m_foreachVars.clear();
    m_userCmds.clear();
    m_colors.clear();
    m_userPics.clear();
    m_userPaths.clear();
    for (const auto &lib : m_snippetLibs)
        if (!lib.trimmed().isEmpty())
            m_activeLibs.insert(lib.trimmed());
    for (const auto &pkg : m_snippetPkgs)
        addPackageImplications(pkg, m_activeLibs);
    m_activeLibs.unite(m_templateLibs);
}

void TikzDocumentState::setSnippetLibraries(const QStringList &libs)
{
    m_snippetLibs = libs;
}

void TikzDocumentState::setSnippetPackages(const QStringList &pkgs)
{
    m_snippetPkgs = pkgs;
}

void TikzDocumentState::setTemplateContent(const QString &content)
{
    m_templateLibs.clear();

    static const QRegularExpression usepackageRe(
        QStringLiteral("\\\\usepackage\\s*(?:\\[[^\\]]*\\]\\s*)?\\{([^}]*)\\}"));
    static const QRegularExpression uselibRe(
        QStringLiteral("\\\\usetikzlibrary\\s*\\{([^}]*)\\}"));

    const QStringList lines = content.split(QLatin1Char('\n'));
    for (const QString &rawLine : lines) {
        // Strip comments: everything from an unescaped '%' onwards.
        QString line = rawLine;
        for (int i = 0; i < line.length(); ++i) {
            if (line.at(i) == QLatin1Char('%')
                && (i == 0 || line.at(i - 1) != QLatin1Char('\\'))) {
                line.truncate(i);
                break;
            }
        }
        if (line.trimmed().isEmpty())
            continue;

        QRegularExpressionMatchIterator pit = usepackageRe.globalMatch(line);
        while (pit.hasNext()) {
            const QString pkgs = pit.next().captured(1);
            for (const QString &pkg : pkgs.split(QLatin1Char(','), Qt::SkipEmptyParts))
                addPackageImplications(pkg, m_templateLibs);
        }

        QRegularExpressionMatchIterator lit = uselibRe.globalMatch(line);
        while (lit.hasNext()) {
            const QString libs = lit.next().captured(1);
            for (const QString &lib : libs.split(QLatin1Char(','), Qt::SkipEmptyParts))
                if (!lib.trimmed().isEmpty())
                    m_templateLibs.insert(lib.trimmed());
        }
    }
}

void TikzDocumentState::reparse(const QTextDocument *doc)
{
    if (!doc) return;
    clear();
    int braceDepth = 0;
    int bracketDepth = 0;
    for (auto block = doc->begin(); block != doc->end(); block = block.next()) {
        QString text = block.text();
        int blockPos = block.position();
        parseLine(text, blockPos, braceDepth, bracketDepth);
    }
    int endPos = doc->characterCount();
    for (auto &scope : m_scopeStack)
        if (scope.endPos < 0) scope.endPos = endPos;
}

void TikzDocumentState::parseLine(const QString &text, int blockStartPos,
                                   int &braceDepth, int &bracketDepth)
{
    int len = text.length();
    int pos = 0;

    // Skip full-line comments
    int trimStart = 0;
    while (trimStart < len && text.at(trimStart) == QLatin1Char(' ')) trimStart++;
    if (trimStart < len && text.at(trimStart) == QLatin1Char('%')) return;

    // Everything before a trailing comment still takes part in the whole-line
    // scans at the end of this function (styles, path-op coordinates/nodes,
    // name paths, ...). A '%' at depth 0 truncates the scanned text instead of
    // aborting the line, so "\draw ... coordinate (A); % note" keeps A.
    int scanLen = len;

    while (pos < len) {
        QChar ch = text.at(pos);

        if (ch == QLatin1Char('{')) { braceDepth++; pos++; continue; }
        if (ch == QLatin1Char('}')) { if (braceDepth > 0) braceDepth--; pos++; continue; }
        if (ch == QLatin1Char('[')) { bracketDepth++; pos++; continue; }
        if (ch == QLatin1Char(']')) { if (bracketDepth > 0) bracketDepth--; pos++; continue; }
        if (ch == QLatin1Char('%') && braceDepth == 0 && bracketDepth == 0) {
            scanLen = pos;
            break;
        }

        if (ch == QLatin1Char('\\') && pos + 1 < len
            && !text.at(pos + 1).isLetter()) {
            // Escaped character (\%, \{, \}, \\, ...): it neither starts a
            // command nor counts for brace/bracket depth or comments.
            pos += 2;
            continue;
        }

        if (ch == QLatin1Char('\\') && pos + 4 < len) {
            // \begin{...}
            QRegularExpressionMatch bm = m_beginRe.match(text, pos);
            if (bm.hasMatch() && bm.capturedStart() == pos) {
                QString envName = bm.captured(1).trimmed().toLower();
                int docPos = blockStartPos + bm.capturedEnd();
                handleBeginScope(envName, docPos, braceDepth);
                pos = bm.capturedEnd();
                continue;
            }

            // \end{...}
            QRegularExpressionMatch em = m_endRe.match(text, pos);
            if (em.hasMatch() && em.capturedStart() == pos) {
                QString envName = em.captured(1).trimmed().toLower();
                int docPos = blockStartPos + pos;
                handleEndScope(envName, docPos);
                pos = em.capturedEnd();
                continue;
            }

            // \usetikzlibrary{...}
            QRegularExpressionMatch ul = m_uselibRe.match(text, pos);
            if (ul.hasMatch() && ul.capturedStart() == pos) {
                QString libs = ul.captured(1);
                for (const QString &lib : libs.split(QLatin1Char(','), Qt::SkipEmptyParts))
                    if (!lib.trimmed().isEmpty())
                        m_activeLibs.insert(lib.trimmed());
                pos = ul.capturedEnd();
                continue;
            }

            // \usepackage{circuitikz} etc.
            QRegularExpressionMatch up = m_usepackageRe.match(text, pos);
            if (up.hasMatch() && up.capturedStart() == pos) {
                QString pkgs = up.captured(1);
                for (const QString &pkg : pkgs.split(QLatin1Char(','), Qt::SkipEmptyParts))
                    addPackageImplications(pkg, m_activeLibs);
                pos = up.capturedEnd();
                continue;
            }

            // \definecolor{name}{model}{spec}
            QRegularExpressionMatch dc = m_definecolorRe.match(text, pos);
            if (dc.hasMatch() && dc.capturedStart() == pos) {
                QString name = dc.captured(1).trimmed();
                int after = pos + dc.capturedLength();
                auto readBrace = [&text, &len](int &p) -> QString {
                    if (p < len && text.at(p) == QLatin1Char('{')) {
                        int start = ++p;
                        int depth = 1;
                        while (p < len && depth > 0) {
                            if (text.at(p) == QLatin1Char('{')) depth++;
                            else if (text.at(p) == QLatin1Char('}')) depth--;
                            if (depth > 0) p++;
                        }
                        if (p < len) p++;
                        return text.mid(start, p - start - 1).trimmed();
                    }
                    return {};
                };
                QString model = readBrace(after);
                QString spec = readBrace(after);
                if (!name.isEmpty() && !spec.isEmpty())
                    m_colors[name] = {model, spec};
                pos = after;
                continue;
            }

            // \colorlet{name}{base}
            QRegularExpressionMatch cl = m_colorletRe.match(text, pos);
            if (cl.hasMatch() && cl.capturedStart() == pos) {
                QString name = cl.captured(1).trimmed();
                if (!name.isEmpty()) m_colors[name] = {"let",""};
                pos = cl.capturedEnd();
                if (pos < len && text.at(pos) == QLatin1Char('{')) {
                    int depth = 1; pos++;
                    while (pos < len && depth > 0) {
                        if (text.at(pos) == QLatin1Char('{')) depth++;
                        else if (text.at(pos) == QLatin1Char('}')) depth--;
                        pos++;
                    }
                }
                continue;
            }

            // \tikzset{...}
            QRegularExpressionMatch ts = m_tikzsetRe.match(text, pos);
            if (ts.hasMatch() && ts.capturedStart() == pos) {
                pos = ts.capturedEnd();
                braceDepth++;
                continue;
            }

            // \tikzstyle{name}
            QRegularExpressionMatch tsty = m_tikzstyleRe.match(text, pos);
            if (tsty.hasMatch() && tsty.capturedStart() == pos) {
                QString styleName = tsty.captured(1).trimmed();
                if (!styleName.isEmpty())
                    m_userStyles[styleName] = QStringLiteral("tikzstyle");
                pos = tsty.capturedEnd();
                continue;
            }

            // \coordinate[...] (name)
            QRegularExpressionMatch coo = m_coordinateRe.match(text, pos);
            if (coo.hasMatch() && coo.capturedStart() == pos) {
                QString name = coo.captured(1).trimmed();
                if (!name.isEmpty()) {
                    m_userCoords.insert(name);
                    m_userNodes.insert(name);
                }
                // name=<name> inside the options (e.g. \coordinate[name=D] (E))
                // is extracted by the brace-aware option scan at the end of
                // this function.
                pos = coo.capturedEnd();
                continue;
            }

            // \foreach \var / \var2 [...] in ...
            QRegularExpressionMatch fe = m_foreachRe.match(text, pos);
            if (fe.hasMatch() && fe.capturedStart() == pos) {
                // Scan the full matched text (including optional brackets) for
                // \variables — varExtractRe will then catch \i inside [count=\i].
                static const QRegularExpression varExtractRe(
                    QStringLiteral("\\\\([a-zA-Z]+)"));
                QRegularExpressionMatchIterator vit =
                    varExtractRe.globalMatch(fe.captured(0));
                while (vit.hasNext()) {
                    QRegularExpressionMatch vm = vit.next();
                    QString varName = vm.captured(1);
                    if (!varName.isEmpty())
                        m_foreachVars.insert(varName);
                }
                pos = fe.capturedEnd();
                continue;
            }

            // \newcommand{\foo}...
            QRegularExpressionMatch ncmd = m_newcmdRe.match(text, pos);
            if (ncmd.hasMatch() && ncmd.capturedStart() == pos) {
                QString cmdText = ncmd.captured();
                int bs = cmdText.lastIndexOf(QLatin1Char('\\'));
                if (bs >= 0) {
                    QString cmdName = cmdText.mid(bs);
                    if (!cmdName.isEmpty()) m_userCmds.insert(cmdName);
                }
                pos = ncmd.capturedEnd();
                continue;
            }

            // \def/\edef/\gdef/\xdef\foo
            QRegularExpressionMatch defm = m_defRe.match(text, pos);
            if (defm.hasMatch() && defm.capturedStart() == pos) {
                QString cmdText = defm.captured();
                int bs = cmdText.lastIndexOf(QLatin1Char('\\'));
                if (bs >= 0) {
                    QString cmdName = cmdText.mid(bs);
                    if (!cmdName.isEmpty()) m_userCmds.insert(cmdName);
                }
                pos = defm.capturedEnd();
                continue;
            }

            // \let\foo=\bar  or  \let\foo\bar
            QRegularExpressionMatch letm = m_letRe.match(text, pos);
            if (letm.hasMatch() && letm.capturedStart() == pos) {
                QString cmdName = QLatin1Char('\\') + letm.captured(1);
                if (!cmdName.isEmpty()) m_userCmds.insert(cmdName);
                pos = letm.capturedEnd();
                continue;
            }

            // \node/\pic/\matrix [...] (name) — any group order, see m_nodeRe.
            QRegularExpressionMatch nm = m_nodeRe.match(text, pos);
            if (nm.hasMatch() && nm.capturedStart() == pos) {
                QString nodeName = nm.captured(1).trimmed();
                if (!nodeName.isEmpty())
                    m_userNodes.insert(nodeName);
                // name=<name> inside the options (e.g. \node[name=B] (C)) is
                // extracted by the brace-aware option scan at the end of this
                // function.
                pos = nm.capturedEnd();
                continue;
            }

            pos++;
            continue;
        }

        pos++;
    }

    // Text taking part in the whole-line scans below: the part of the line
    // before a trailing depth-0 comment (or the full line when there is none).
    const QString scanText = (scanLen == len) ? text : text.left(scanLen);

    // Scan for style definitions anywhere in the text (handles multi-line \tikzset)
    QRegularExpressionMatchIterator sit = m_styleInTikzsetRe.globalMatch(scanText);
    while (sit.hasNext()) {
        QRegularExpressionMatch sm = sit.next();
        QString sName = sm.captured(1).trimmed();
        QString kind = sm.captured(2);
        if (kind == QLatin1String("pic"))
            m_userPics.insert(sName);
        else
            m_userStyles[sName] = kind;
    }

    // Scan for the path-operation coordinate form 'coordinate (name)' used
    // inside a path, e.g. \draw (2,1) coordinate (test) circle [radius=2mm];
    // (The \coordinate command form is already handled above.)
    QRegularExpressionMatchIterator coit = m_coordOpRe.globalMatch(scanText);
    while (coit.hasNext()) {
        QRegularExpressionMatch cm = coit.next();
        QString name = cm.captured(1).trimmed();
        if (!name.isEmpty()) {
            m_userCoords.insert(name);
            m_userNodes.insert(name);
        }
    }

    // Scan for the path-operation node/pic form 'node [opts] (name)' used
    // inside a path, e.g.
    //   \draw (0,0) to[short,o-] ++(1,0) node[op amp] (OA) {OA1};
    // (The \node/\pic command forms are already handled above.)
    QRegularExpressionMatchIterator noit = m_nodeOpRe.globalMatch(scanText);
    while (noit.hasNext()) {
        QRegularExpressionMatch nm = noit.next();
        QString name = nm.captured(1).trimmed();
        if (!name.isEmpty())
            m_userNodes.insert(name);
    }

    // Scan for name=/n= keys inside the option groups of node/pic/coordinate/
    // matrix/to operations and axis-like environments.
    extractOptionNames(scanText);

    // Scan for named paths: name path[ global|local]=<name>. These feed the
    // 'of=' completion inside \path[name intersections={of=A and B,...}].
    QRegularExpressionMatchIterator pit = m_namePathRe.globalMatch(scanText);
    while (pit.hasNext()) {
        QRegularExpressionMatch pm = pit.next();
        QString pName = pm.captured(1);
        if (pName.isEmpty()) pName = pm.captured(2);
        pName = pName.trimmed();
        if (!pName.isEmpty())
            m_userPaths.insert(pName);
    }

    // Scan for by={...} coordinate names inside name intersections. Each entry
    // may carry an optional [options] prefix, e.g. by={[label=95:$L$]L,H} names
    // the intersection coordinates L and H.
    QRegularExpressionMatchIterator bit = m_byRe.globalMatch(scanText);
    while (bit.hasNext()) {
        QRegularExpressionMatch bm = bit.next();
        const QString inner = bm.captured(1);
        // Split on commas at brace/bracket depth 0.
        int depth = 0;
        QString token;
        auto flush = [&]() {
            QString t = token.trimmed();
            // Strip a leading [..] option group.
            if (t.startsWith(QLatin1Char('['))) {
                int close = t.indexOf(QLatin1Char(']'));
                if (close >= 0) t = t.mid(close + 1).trimmed();
            }
            if (!t.isEmpty()) {
                m_userCoords.insert(t);
                m_userNodes.insert(t);
            }
            token.clear();
        };
        for (int i = 0; i < inner.length(); ++i) {
            const QChar c = inner.at(i);
            if (c == QLatin1Char('{') || c == QLatin1Char('[')) depth++;
            else if (c == QLatin1Char('}') || c == QLatin1Char(']')) { if (depth > 0) depth--; }
            if (c == QLatin1Char(',') && depth == 0) { flush(); continue; }
            token.append(c);
        }
        flush();
    }
}

namespace {
// Index of the ']' closing the '[' at openIdx, honoring brace nesting so
// brackets inside {...} values (e.g. label={[red]above:x}) do not close the
// group. Returns -1 when the group is not closed on this line.
int matchingBracketEnd(const QString &text, int openIdx)
{
    int brace = 0;
    for (int i = openIdx + 1; i < text.length(); ++i) {
        const QChar c = text.at(i);
        if (c == QLatin1Char('{')) brace++;
        else if (c == QLatin1Char('}')) { if (brace > 0) brace--; }
        else if (brace == 0 && c == QLatin1Char(']')) return i;
    }
    return -1;
}
} // namespace

void TikzDocumentState::extractOptionNames(const QString &text)
{
    // Operations whose option groups may carry a name= key. Verified against
    // the TeXLive sources:
    //  - /tikz/name applies to node/pic/coordinate (tikz.code.tex; \matrix is
    //    literally \path node[matrix]),
    //  - circuitikz maps name=/n= inside to[...] to bipole/name and creates
    //    the extra coordinates <name>start and <name>end (pgfcircpath.tex),
    //  - pgfplots' /pgfplots/name forwards to /tikz/name, so
    //    \begin{axis}[name=ax] names the axis node (pgfplots.code.tex).
    static const QRegularExpression opKwRe(QStringLiteral(
        "(?<![a-zA-Z@\\\\])\\\\?(node|pic|coordinate|matrix|to)(?![a-zA-Z])"
        "|\\\\begin\\s*\\{\\s*(axis|semilogxaxis|semilogyaxis|loglogaxis|"
        "polaraxis|ternaryaxis|smithchart|groupplot)\\s*\\}"));

    const int n = text.length();
    QRegularExpressionMatchIterator kit = opKwRe.globalMatch(text);
    while (kit.hasNext()) {
        const QRegularExpressionMatch km = kit.next();
        const bool isEnv = km.captured(1).isEmpty();
        const QString kw = isEnv ? km.captured(2) : km.captured(1);
        const bool circuitikzActive =
            m_activeLibs.contains(QStringLiteral("circuitikz"));

        // Walk the operation's argument sequence: any mix of [options],
        // (name) and 'at (coord)' groups (tikz.code.tex allows any order).
        int p = km.capturedEnd();
        while (p < n) {
            while (p < n && text.at(p).isSpace()) p++;
            if (p >= n) break;
            const QChar c = text.at(p);
            if (c == QLatin1Char('[')) {
                const int close = matchingBracketEnd(text, p);
                if (close < 0) break;
                const QString content = text.mid(p + 1, close - p - 1);

                // Split on commas at brace/bracket depth 0 and look for
                // name=<value> (and, for circuitikz to[...], n=<value>).
                int depth = 0;
                QString part;
                const auto handlePart = [&]() {
                    static const QRegularExpression nameKeyRe(QStringLiteral(
                        "^\\s*(name|n)\\s*=\\s*(.*)$"));
                    const QRegularExpressionMatch nv = nameKeyRe.match(part);
                    part.clear();
                    if (!nv.hasMatch()) return;
                    const bool shorthand = nv.captured(1) == QLatin1String("n");
                    // 'n' is the circuitikz shorthand and only valid there.
                    if (shorthand
                        && !(kw == QLatin1String("to") && circuitikzActive))
                        return;
                    QString value = nv.captured(2).trimmed();
                    if (value.startsWith(QLatin1Char('{'))
                        && value.endsWith(QLatin1Char('}')))
                        value = value.mid(1, value.length() - 2).trimmed();
                    if (value.isEmpty()) return;
                    m_userNodes.insert(value);
                    if (kw == QLatin1String("coordinate"))
                        m_userCoords.insert(value);
                    // circuitikz names a bipole node <name> and additionally
                    // creates the coordinates <name>start and <name>end.
                    if (kw == QLatin1String("to") && circuitikzActive) {
                        m_userCoords.insert(value + QLatin1String("start"));
                        m_userCoords.insert(value + QLatin1String("end"));
                        m_userNodes.insert(value + QLatin1String("start"));
                        m_userNodes.insert(value + QLatin1String("end"));
                    }
                };
                for (int i = 0; i < content.length(); ++i) {
                    const QChar cc = content.at(i);
                    if (cc == QLatin1Char('{') || cc == QLatin1Char('['))
                        depth++;
                    else if (cc == QLatin1Char('}') || cc == QLatin1Char(']')) {
                        if (depth > 0) depth--;
                    }
                    if (cc == QLatin1Char(',') && depth == 0) {
                        handlePart();
                        continue;
                    }
                    part.append(cc);
                }
                handlePart();

                p = close + 1;
                if (isEnv) break;      // env options: a single bracket group
            } else if (!isEnv && c == QLatin1Char('(')) {
                const int close = text.indexOf(QLatin1Char(')'), p);
                if (close < 0) break;
                p = close + 1;
            } else if (!isEnv && c == QLatin1Char('a')
                       && text.mid(p, 2) == QLatin1String("at")
                       && (p + 2 >= n
                           || !text.at(p + 2).isLetterOrNumber())) {
                p += 2;
            } else {
                break;
            }
        }
    }
}

void TikzDocumentState::handleBeginScope(const QString &envName, int pos, int braceDepth)
{
    if (!m_tikzEnvSet.contains(envName)) return;
    // Environments imply completion libraries: writing \begin{circuitikz}
    // means circuitikz completions must be available even when the snippet
    // itself carries no \usepackage line (the wrapper template provides the
    // package at compile time). Same for the pgfplots axis family, tikz-cd
    // and tikz-feynman.
    if (envName == QLatin1String("circuitikz")) {
        m_activeLibs.insert(QStringLiteral("circuitikz"));
    } else if (envName == QLatin1String("tikzcd")) {
        m_activeLibs.insert(QStringLiteral("cd"));
    } else if (envName == QLatin1String("feynman")) {
        m_activeLibs.insert(QStringLiteral("tikz-feynman"));
    } else if (envName == QLatin1String("axis")
               || envName == QLatin1String("semilogxaxis")
               || envName == QLatin1String("semilogyaxis")
               || envName == QLatin1String("loglogaxis")
               || envName == QLatin1String("polaraxis")
               || envName == QLatin1String("ternaryaxis")
               || envName == QLatin1String("smithchart")
               || envName == QLatin1String("groupplot")) {
        m_activeLibs.insert(QStringLiteral("pgfplots"));
    }
    Scope scope;
    scope.env = envName;
    scope.startPos = pos;
    scope.endPos = -1;
    scope.braceDepth = braceDepth;
    m_scopeStack.append(scope);
}

void TikzDocumentState::handleEndScope(const QString &envName, int pos)
{
    for (int i = m_scopeStack.size() - 1; i >= 0; i--) {
        if (m_scopeStack[i].env == envName && m_scopeStack[i].endPos < 0) {
            m_scopeStack[i].endPos = pos;
            return;
        }
    }
}

const TikzDocumentState::Scope *TikzDocumentState::currentScope(int position) const
{
    for (int i = m_scopeStack.size() - 1; i >= 0; i--) {
        if (m_scopeStack[i].startPos <= position &&
            (m_scopeStack[i].endPos < 0 || m_scopeStack[i].endPos > position))
            return &m_scopeStack[i];
    }
    return nullptr;
}

QString TikzDocumentState::currentEnvName(int position) const
{
    const Scope *s = currentScope(position);
    return s ? s->env : QString();
}

const TikzDocumentState::CommandContext *TikzDocumentState::currentCommand(int) const
{
    return nullptr;
}

QString TikzDocumentState::currentCmdName(int) const
{
    return QString();
}

QStringList TikzDocumentState::allUserDefinitions() const
{
    QStringList result;
    for (const auto &k : m_userStyles.keys()) result << k;
    for (const auto &c : m_userCoords) result << c;
    for (const auto &n : m_userNodes) result << n;
    for (const auto &v : m_foreachVars) result << (QLatin1String("\\") + v);
    for (const auto &c : m_userCmds) result << c;
    for (const auto &k : m_colors.keys()) result << k;
    for (const auto &p : m_userPics) result << p;
    return result;
}
