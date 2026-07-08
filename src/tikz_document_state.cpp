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
    m_coordinateRe = QRegularExpression(
        QStringLiteral("\\\\coordinate\\s*(?:\\[[^\\]]*\\]\\s*)?\\(([^)]+)\\)"));
    m_foreachRe = QRegularExpression(
        QStringLiteral("\\\\foreach\\s+((?:\\\\[a-zA-Z]+\\s*(?:/\\s*)?)+)in"));
    m_newcmdRe = QRegularExpression(
        QStringLiteral("\\\\(?:new|renew|provide)command\\*?\\s*\\{?\\\\([a-zA-Z@]+)"));
    m_defRe = QRegularExpression(QStringLiteral("\\\\def\\s*\\\\([a-zA-Z]+)"));
    m_nodeRe = QRegularExpression(
        QStringLiteral("\\\\(?:node|pic)\\s*(?:\\[[^\\]]*\\]\\s*)?"
                       "(?:at\\s*\\([^)]*\\)\\s*)?\\(([^)]+)\\)"));
    m_styleInTikzsetRe = QRegularExpression(
        QStringLiteral("([\\w\\s]+)/\\.(style|code|pic|append style|prefix style)"
                       "\\s*=\\s*\\{"));
    m_commentRe = QRegularExpression(QStringLiteral("%"));
    m_usepackageRe = QRegularExpression(
        QStringLiteral("\\\\usepackage\\s*(?:\\[[^\\]]*\\]\\s*)?\\{([^}]*)\\}"));

    m_tikzEnvSet = {
        "tikzpicture","scope","pgfonlayer",
        "axis","semilogxaxis","semilogyaxis","loglogaxis",
        "polaraxis","smithchart","ternaryaxis","groupplot",
        "tikzcd","circuitikz","forest","mindmap"
    };
}

TikzDocumentState::~TikzDocumentState() = default;

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
    for (const auto &lib : m_snippetLibs)
        if (!lib.trimmed().isEmpty())
            m_activeLibs.insert(lib.trimmed());
    for (const auto &pkg : m_snippetPkgs) {
        if (pkg.contains("circuitikz", Qt::CaseInsensitive))
            m_activeLibs.insert("circuitikz");
        if (pkg.contains("tikz-3dplot", Qt::CaseInsensitive))
            m_activeLibs.insert("3d");
        if (pkg.contains("tikz-cd", Qt::CaseInsensitive))
            m_activeLibs.insert("cd");
    }
}

void TikzDocumentState::setSnippetLibraries(const QStringList &libs)
{
    m_snippetLibs = libs;
}

void TikzDocumentState::setSnippetPackages(const QStringList &pkgs)
{
    m_snippetPkgs = pkgs;
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

    while (pos < len) {
        QChar ch = text.at(pos);

        if (ch == QLatin1Char('{')) { braceDepth++; pos++; continue; }
        if (ch == QLatin1Char('}')) { if (braceDepth > 0) braceDepth--; pos++; continue; }
        if (ch == QLatin1Char('[')) { bracketDepth++; pos++; continue; }
        if (ch == QLatin1Char(']')) { if (bracketDepth > 0) bracketDepth--; pos++; continue; }
        if (ch == QLatin1Char('%') && braceDepth == 0 && bracketDepth == 0) return;

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
                for (const QString &pkg : pkgs.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
                    QString trimmed = pkg.trimmed().toLower();
                    if (!trimmed.isEmpty()) {
                        if (trimmed == QLatin1String("circuitikz"))
                            m_activeLibs.insert("circuitikz");
                        if (trimmed == QLatin1String("tikz-3dplot"))
                            m_activeLibs.insert("3d");
                        if (trimmed == QLatin1String("tikz-cd"))
                            m_activeLibs.insert("cd");
                        if (trimmed == QLatin1String("tkz-euclide"))
                            m_activeLibs.insert("tkz-euclide");
                    }
                }
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
                pos = coo.capturedEnd();
                continue;
            }

            // \foreach \var / \var2 in ...
            QRegularExpressionMatch fe = m_foreachRe.match(text, pos);
            if (fe.hasMatch() && fe.capturedStart() == pos) {
                QString varsStr = fe.captured(1);
                static const QRegularExpression varExtractRe(QStringLiteral("\\\\([a-zA-Z]+)"));
                QRegularExpressionMatchIterator vit = varExtractRe.globalMatch(varsStr);
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

            // \def\foo
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

            // \node[...] (name)
            QRegularExpressionMatch nm = m_nodeRe.match(text, pos);
            if (nm.hasMatch() && nm.capturedStart() == pos) {
                QString nodeName = nm.captured(1).trimmed();
                if (!nodeName.isEmpty())
                    m_userNodes.insert(nodeName);
                pos = nm.capturedEnd();
                continue;
            }

            pos++;
            continue;
        }

        pos++;
    }

    // Scan for style definitions anywhere in the text (handles multi-line \tikzset)
    QRegularExpressionMatchIterator sit = m_styleInTikzsetRe.globalMatch(text);
    while (sit.hasNext()) {
        QRegularExpressionMatch sm = sit.next();
        QString sName = sm.captured(1).trimmed();
        QString kind = sm.captured(2);
        if (kind == QLatin1String("pic"))
            m_userPics.insert(sName);
        else
            m_userStyles[sName] = kind;
    }
}

void TikzDocumentState::handleBeginScope(const QString &envName, int pos, int braceDepth)
{
    if (!m_tikzEnvSet.contains(envName)) return;
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
