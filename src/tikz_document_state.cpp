#include "tikz_document_state.h"
#include <QTextBlock>
#include <QTextDocument>
#include <QRegularExpressionMatch>

TikzDocumentState::TikzDocumentState()
{
    m_beginRe = QRegularExpression(QStringLiteral("\\\\begin\\s*\\{([^}]*)\\}"));
    m_endRe = QRegularExpression(QStringLiteral("\\\\end\\s*\\{([^}]*)\\}"));
    m_cmdRe = QRegularExpression(QStringLiteral("\\\\([a-zA-Z@][a-zA-Z@]*)\\b"));
    m_uselibRe = QRegularExpression(QStringLiteral("\\\\usetikzlibrary\\s*\\{([^}]*)\\}"));
    m_tikzsetRe = QRegularExpression(QStringLiteral("\\\\tikzset\\s*\\{"));
    m_tikzstyleRe = QRegularExpression(QStringLiteral("\\\\tikzstyle\\s*\\{([^}]*)\\}"));
    m_definecolorRe = QRegularExpression(QStringLiteral("\\\\definecolor\\s*\\{([^}]*)\\}"));
    m_colorletRe = QRegularExpression(QStringLiteral("\\\\colorlet\\s*\\{([^}]*)\\}"));
    m_coordinateRe = QRegularExpression(QStringLiteral("\\\\coordinate\\s*(?:\\[.*?\\]\\s*)?\\(([^)]+)\\)"));
    m_foreachRe = QRegularExpression(QStringLiteral("\\\\foreach\\s+(\\\\([a-zA-Z]+)(?:/\\\\([a-zA-Z]+))?\\s+in"));
    m_newcmdRe = QRegularExpression(
        QStringLiteral("\\\\(?:new|renew|provide)command\\*?\\s*\\{?\\\\([a-zA-Z@]+)"));
    m_defRe = QRegularExpression(QStringLiteral("\\\\def\\s*\\\\([a-zA-Z]+)"));
    m_nodeRe = QRegularExpression(QStringLiteral("\\\\node(?:\\[[^\\]]*\\])?\\s*(?:\\(([^)]+)\\))?"));
    m_commentRe = QRegularExpression(QStringLiteral("(?<!\\\\)%"));

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
    m_cmdContexts.clear();
    m_activeLibs.clear();
    m_userStyles.clear();
    m_userCoords.clear();
    m_userNodes.clear();
    m_foreachVars.clear();
    m_userCmds.clear();
    m_colors.clear();
    m_userPics.clear();
    m_blockScopeDepth.clear();
    m_blockBraceDepth.clear();
    m_totalBlocks = 0;

    // Add snippet libraries
    for (const auto &lib : m_snippetLibs) {
        QString cleaned = lib.trimmed();
        if (!cleaned.isEmpty())
            m_activeLibs.insert(cleaned);
    }
    // Package implies library availability
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

    int numBlocks = doc->blockCount();
    m_blockScopeDepth.resize(numBlocks);
    m_blockBraceDepth.resize(numBlocks);
    m_totalBlocks = numBlocks;

    int braceDepth = 0;
    int bracketDepth = 0;
    bool inComment = false;

    for (int i = 0; i < numBlocks; i++) {
        QTextBlock block = doc->findBlockByNumber(i);
        if (!block.isValid()) continue;

        m_blockScopeDepth[i] = m_scopeStack.size();
        m_blockBraceDepth[i] = braceDepth;

        QString text = block.text();
        int blockStartPos = block.position();
        parseLine(text, blockStartPos, braceDepth, bracketDepth, inComment);
    }

    // Close remaining open scopes
    int endPos = doc->characterCount();
    for (auto &scope : m_scopeStack) {
        if (scope.endPos < 0)
            scope.endPos = endPos;
    }
}

void TikzDocumentState::reparseBlock(const QTextBlock &block, int blockPos)
{
    Q_UNUSED(block)
    Q_UNUSED(blockPos)
}

void TikzDocumentState::parseLine(const QString &text, int blockStartPos,
                                   int &braceDepth, int &bracketDepth, bool &inComment)
{
    int pos = 0;
    int len = text.length();

    while (pos < len) {
        // Handle comments
        QRegularExpressionMatch cm = m_commentRe.match(text, pos);
        if (cm.hasMatch() && cm.capturedStart() == pos) {
            // The rest of this line is a comment
            return;
        }
        // Skip leading whitespace
        if (pos < len && (text.at(pos) == ' ' || text.at(pos) == '\t')) {
            pos++;
            continue;
        }

        QChar ch = text.at(pos);

        // Track brace depth
        if (ch == '{') {
            braceDepth++;
            pos++;
            continue;
        }
        if (ch == '}') {
            if (braceDepth > 0) braceDepth--;
            pos++;
            continue;
        }
        if (ch == '[') {
            bracketDepth++;
            pos++;
            continue;
        }
        if (ch == ']') {
            if (bracketDepth > 0) bracketDepth--;
            pos++;
            continue;
        }

        // Check for \begin{...}
        if (ch == '\\' && pos + 5 < len) {
            QRegularExpressionMatch bm = m_beginRe.match(text, pos);
            if (bm.hasMatch() && bm.capturedStart() == pos) {
                QString envName = bm.captured(1).trimmed().toLower();
                int docPos = blockStartPos + pos;
                handleBeginScope(envName, docPos, braceDepth);
                pos += bm.capturedLength();
                continue;
            }

            QRegularExpressionMatch em = m_endRe.match(text, pos);
            if (em.hasMatch() && em.capturedStart() == pos) {
                QString envName = em.captured(1).trimmed().toLower();
                int docPos = blockStartPos + pos + em.capturedLength();
                handleEndScope(envName, docPos);
                pos += em.capturedLength();
                continue;
            }

            // \usetikzlibrary{...}
            QRegularExpressionMatch ul = m_uselibRe.match(text, pos);
            if (ul.hasMatch() && ul.capturedStart() == pos) {
                QString libs = ul.captured(1);
                for (const QString &lib : libs.split(',', Qt::SkipEmptyParts)) {
                    QString cleaned = lib.trimmed();
                    if (!cleaned.isEmpty())
                        m_activeLibs.insert(cleaned);
                }
                pos += ul.capturedLength();
                continue;
            }

            // \definecolor{name}{model}{spec}
            QRegularExpressionMatch dc = m_definecolorRe.match(text, pos);
            if (dc.hasMatch() && dc.capturedStart() == pos) {
                QString name = dc.captured(1).trimmed();
                // try to read the rest of the color spec
                int after = pos + dc.capturedLength();
                auto readBrace = [&](int &p) -> QString {
                    if (p < len && text.at(p) == '{') {
                        int start = p + 1;
                        int depth = 1;
                        p++;
                        while (p < len && depth > 0) {
                            if (text.at(p) == '{') depth++;
                            else if (text.at(p) == '}') depth--;
                            p++;
                        }
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
                if (!name.isEmpty())
                    m_colors[name] = {"let",""};
                pos += cl.capturedLength();
                // skip next brace
                int after = pos;
                if (after < len && text.at(after) == '{') {
                    int depth = 1;
                    after++;
                    while (after < len && depth > 0) {
                        if (text.at(after) == '{') depth++;
                        else if (text.at(after) == '}') depth--;
                        after++;
                    }
                    pos = after;
                }
                continue;
            }

            // \tikzset{...} — extract style definitions
            QRegularExpressionMatch ts = m_tikzsetRe.match(text, pos);
            if (ts.hasMatch() && ts.capturedStart() == pos) {
                int start = pos + ts.capturedLength();
                pos = start;
                // Parse content of tikzset to find style definitions
                static const QRegularExpression styleRe(
                    QStringLiteral("(\\w+)/\\.(style|code|pic|append style|prefix style)\\s*=\\s*\\{"));
                QRegularExpressionMatchIterator sit = styleRe.globalMatch(text.mid(start));
                while (sit.hasNext()) {
                    QRegularExpressionMatch sm = sit.next();
                    QString styleName = sm.captured(1);
                    QString kind = sm.captured(2);
                    if (kind == "pic")
                        m_userPics.insert(styleName);
                    else
                        m_userStyles[styleName] = kind;
                }
                // skip to closing brace
                int depth = 1;
                while (pos < len && depth > 0) {
                    if (text.at(pos) == '{') depth++;
                    else if (text.at(pos) == '}') depth--;
                    pos++;
                }
                continue;
            }

            // \tikzstyle{name}+=[...]
            QRegularExpressionMatch tsty = m_tikzstyleRe.match(text, pos);
            if (tsty.hasMatch() && tsty.capturedStart() == pos) {
                QString styleName = tsty.captured(1).trimmed();
                if (!styleName.isEmpty())
                    m_userStyles[styleName] = "tikzstyle";
                pos += tsty.capturedLength();
                continue;
            }

            // \coordinate[...] (name) at ...
            QRegularExpressionMatch coo = m_coordinateRe.match(text, pos);
            if (coo.hasMatch() && coo.capturedStart() == pos) {
                QString name = coo.captured(1).trimmed();
                if (!name.isEmpty()) {
                    m_userCoords.insert(name);
                    m_userNodes.insert(name);
                }
                pos += coo.capturedLength();
                continue;
            }

            // \foreach \var in ...
            QRegularExpressionMatch fe = m_foreachRe.match(text, pos);
            if (fe.hasMatch() && fe.capturedStart() == pos) {
                QString var1 = fe.captured(2);
                if (!var1.isEmpty()) m_foreachVars.insert(var1);
                QString var2 = fe.captured(3);
                if (!var2.isEmpty()) m_foreachVars.insert(var2);
                pos += fe.capturedLength();
                continue;
            }

            // \newcommand{\foo}...
            QRegularExpressionMatch ncmd = m_newcmdRe.match(text, pos);
            if (ncmd.hasMatch() && ncmd.capturedStart() == pos) {
                QString cmdText = ncmd.captured();
                int bs = cmdText.lastIndexOf('\\');
                if (bs >= 0) {
                    QString cmdName = cmdText.mid(bs);
                    if (!cmdName.isEmpty()) m_userCmds.insert(cmdName);
                }
                pos += ncmd.capturedLength();
                continue;
            }

            // \def\foo ...
            QRegularExpressionMatch defm = m_defRe.match(text, pos);
            if (defm.hasMatch() && defm.capturedStart() == pos) {
                QString cmdText = defm.captured();
                int bs = cmdText.lastIndexOf('\\');
                if (bs >= 0) {
                    QString cmdName = cmdText.mid(bs);
                    if (!cmdName.isEmpty()) m_userCmds.insert(cmdName);
                }
                pos += defm.capturedLength();
                continue;
            }

            // \node[...] (name) ...
            QRegularExpressionMatch nm = m_nodeRe.match(text, pos);
            if (nm.hasMatch() && nm.capturedStart() == pos) {
                QString nodeName = nm.captured(1).trimmed();
                if (!nodeName.isEmpty())
                    m_userNodes.insert(nodeName);
                pos += nm.capturedLength();

                // Detect ongoing [options] or (name) for command context
                if (m_tikzEnvSet.contains(currentEnvName(blockStartPos + pos))) {
                    CommandContext ctx;
                    ctx.name = "node";
                    ctx.nodeName = nodeName;
                    ctx.startPos = blockStartPos + nm.capturedStart();
                    m_cmdContexts.append(ctx);
                }
                continue;
            }

            // General command detection
            QRegularExpressionMatch cmdMatch = m_cmdRe.match(text, pos);
            if (cmdMatch.hasMatch() && cmdMatch.capturedStart() == pos) {
                QString cmdName = cmdMatch.captured(1).toLower();
                pos += cmdMatch.capturedLength();

                // Drawing commands that have options
                if (cmdName == "draw" || cmdName == "path" || cmdName == "fill" ||
                    cmdName == "filldraw" || cmdName == "shade" || cmdName == "shadedraw" ||
                    cmdName == "node" || cmdName == "pic" || cmdName == "edge" ||
                    cmdName == "scope" || cmdName == "pgfscope" || cmdName == "graph") {

                    CommandContext ctx;
                    ctx.name = cmdName;
                    ctx.startPos = blockStartPos + cmdMatch.capturedStart();
                    m_cmdContexts.append(ctx);
                }
                continue;
            }
        }

        // Not a backslash — advance
        pos++;
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
    scope.options.clear();
    m_scopeStack.append(scope);
}

void TikzDocumentState::handleEndScope(const QString &envName, int pos)
{
    Q_UNUSED(envName)
    if (m_scopeStack.isEmpty()) return;

    // Pop scopes until we find a matching open
    int depth = m_scopeStack.size();
    for (int i = depth - 1; i >= 0; i--) {
        if (m_scopeStack[i].env == envName || i == depth - 1) {
            m_scopeStack[i].endPos = pos;
            m_scopeStack.remove(i);
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

const TikzDocumentState::Scope *TikzDocumentState::currentEnvironmentScope(int position) const
{
    // Find the innermost tikz-like env scope at this position
    for (int i = m_scopeStack.size() - 1; i >= 0; i--) {
        if (m_tikzEnvSet.contains(m_scopeStack[i].env) &&
            m_scopeStack[i].startPos <= position &&
            (m_scopeStack[i].endPos < 0 || m_scopeStack[i].endPos > position))
            return &m_scopeStack[i];
    }
    // Also check closed scopes that contain this position
    // (The scope stack only has open scopes, closed ones are removed)
    return nullptr;
}

QString TikzDocumentState::currentEnvName(int position) const
{
    const Scope *s = currentScope(position);
    return s ? s->env : QString();
}

const TikzDocumentState::CommandContext *TikzDocumentState::currentCommand(int position) const
{
    // Find the command with the closest startPos <= position
    const CommandContext *best = nullptr;
    for (const auto &ctx : m_cmdContexts) {
        if (ctx.startPos <= position) {
            if (!best || ctx.startPos > best->startPos)
                best = &ctx;
        }
    }
    return best;
}

QString TikzDocumentState::currentCmdName(int position) const
{
    const CommandContext *ctx = currentCommand(position);
    return ctx ? ctx->name : QString();
}

QStringList TikzDocumentState::allUserDefinitions() const
{
    QStringList result;
    for (const auto &s : m_userStyles.keys())
        result << s;
    for (const auto &c : m_userCoords)
        result << c;
    for (const auto &n : m_userNodes)
        result << n;
    for (const auto &v : m_foreachVars)
        result << ("\\" + v);
    for (const auto &c : m_userCmds)
        result << c;
    for (const auto &k : m_colors.keys())
        result << k;
    for (const auto &p : m_userPics)
        result << p;
    return result;
}
