#include "tikz_highlighter.h"
#include "tikz_document_state.h"

TikzHighlighter::TikzHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    m_commentFormat.setForeground(QColor(150, 150, 150));
    m_commentFormat.setFontItalic(true);

    m_commandFormat.setForeground(QColor(40, 80, 180));
    m_commandFormat.setFontWeight(QFont::Bold);

    m_envFormat.setForeground(QColor(120, 20, 140));
    m_envFormat.setFontWeight(QFont::Bold);

    m_mathFormat.setForeground(QColor(30, 140, 30));

    m_braceFormat.setForeground(QColor(200, 30, 30));

    m_coordFormat.setForeground(QColor(200, 130, 0));

    m_optionFormat.setForeground(QColor(0, 120, 120));

    m_stringFormat.setForeground(QColor(180, 120, 0));

    m_numberFormat.setForeground(QColor(160, 80, 180));

    m_paramFormat.setForeground(QColor(60, 140, 60));
    m_paramFormat.setFontItalic(true);

    // New formats
    m_userStyleFormat.setForeground(QColor(200, 80, 20));
    m_userStyleFormat.setFontWeight(QFont::Bold);

    m_nodeNameFormat.setForeground(QColor(100, 40, 160));
    m_nodeNameFormat.setFontWeight(QFont::Bold);

    m_coordNameFormat.setForeground(QColor(180, 120, 20));

    m_foreachVarFormat.setForeground(QColor(80, 140, 40));
    m_foreachVarFormat.setFontItalic(true);

    m_userCmdFormat.setForeground(QColor(20, 120, 160));
    m_userCmdFormat.setFontWeight(QFont::Bold);

    m_keyFormat.setForeground(QColor(20, 100, 100));
    m_keyFormat.setFontWeight(QFont::Bold);

    m_pgfPathFormat.setForeground(QColor(80, 40, 140));

    m_handlerFormat.setForeground(QColor(180, 60, 120));

    m_libFormat.setForeground(QColor(120, 80, 40));

    m_rules = {
        {QRegularExpression(QStringLiteral("%[^\n]*")), m_commentFormat, 100},
        {QRegularExpression(QStringLiteral("\"[^\"]*\"")), m_stringFormat, 1},
        {QRegularExpression(QStringLiteral("@@[a-zA-Z_][a-zA-Z0-9_]*@@")), m_paramFormat, 2},
        {QRegularExpression(QStringLiteral("\\\\begin\\{[^}]*\\}|\\\\end\\{[^}]*\\}")),
            m_envFormat, 3},
        {QRegularExpression(QStringLiteral("\\\\[a-zA-Z@][a-zA-Z@]*(\\s*\\*)?")),
            m_commandFormat, 4},
        {QRegularExpression(QStringLiteral("\\$[^\\$]*\\$")), m_mathFormat, 5},
        {QRegularExpression(QStringLiteral("\\\\\\(.*?\\\\\\)")), m_mathFormat, 6},
        {QRegularExpression(QString::fromUtf8(R"(\\\[.*?\\\])")), m_mathFormat, 7},
        {QRegularExpression(QStringLiteral("\\{[^}]*\\}|\\}")), m_braceFormat, 8},
        {QRegularExpression(QStringLiteral("\\(-?[\\d.]+\\.?\\s*,\\s*-?[\\d.]+\\s*\\)")),
            m_coordFormat, 9},
        {QRegularExpression(QStringLiteral("\\[[^\\]]*\\]")), m_optionFormat, 10},
        {QRegularExpression(
            QStringLiteral("\\b\\d+\\.?\\d*(pt|cm|mm|in|ex|em|bp|dd|pc|cc|sp)?\\b")),
            m_numberFormat, 11},

        // New rules
        {QRegularExpression(QStringLiteral("/(tikz|pgf|pgfplots|circuitikz)(/\\w+)*")),
            m_pgfPathFormat, 12},
        {QRegularExpression(QStringLiteral("/\\.(style|code|default|initial|add|store in"
            "|estore in|value required|value forbidden|try|retry|search also|cd|handler|append style"
            "|prefix style)\\b")),
            m_handlerFormat, 13},
        {QRegularExpression(
            QStringLiteral("\\\\usetikzlibrary\\s*\\{[^}]*\\}|\\\\usepgfplotslibrary\\s*\\{[^}]*\\}")),
            m_libFormat, 14},
    };

    std::sort(m_rules.begin(), m_rules.end(),
              [](const Rule &a, const Rule &b) { return a.priority < b.priority; });
}

void TikzHighlighter::setDocumentState(TikzDocumentState *state)
{
    m_docState = state;
}

void TikzHighlighter::highlightBlock(const QString &text)
{
    int prevState = previousBlockState();
    if (prevState == InComment) {
        setFormat(0, text.length(), m_commentFormat);
        static const QRegularExpression endCommentRe(
            QString::fromUtf8(R"(\\end\{comment\})"));
        QRegularExpressionMatch em = endCommentRe.match(text);
        bool hasEnd = em.hasMatch();
        setCurrentBlockState(hasEnd ? Normal : InComment);
        if (!hasEnd) return;
        int after = em.capturedStart() + em.capturedLength();
        if (after < text.length())
            applyRules(text.mid(after));
        return;
    }

    if (text.trimmed().startsWith('%')) {
        setFormat(0, text.length(), m_commentFormat);
        setCurrentBlockState(Normal);
        return;
    }

    applyRules(text);
    applyUserHighlights(text);
    applyKeyValueHighlight(text);

    static const QRegularExpression beginCommentRe(
        QString::fromUtf8(R"(\\begin\{comment\})"));
    QRegularExpressionMatchIterator it = beginCommentRe.globalMatch(text);
    bool endsInComment = false;
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        static const QRegularExpression endCommentRe(
            QString::fromUtf8(R"(\\end\{comment\})"));
        QString after = text.mid(m.capturedEnd());
        if (!endCommentRe.match(after).hasMatch()) {
            setFormat(m.capturedStart(), text.length() - m.capturedStart(), m_commentFormat);
            endsInComment = true;
            break;
        }
    }
    setCurrentBlockState(endsInComment ? InComment : Normal);
}

void TikzHighlighter::applyRules(const QString &text)
{
    for (const Rule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

void TikzHighlighter::applyUserHighlights(const QString &text)
{
    if (!m_docState) return;

    // Highlight user styles
    for (auto it = m_docState->userStyles().cbegin();
         it != m_docState->userStyles().cend(); ++it) {
        highlightWord(text, it.key(), m_userStyleFormat);
    }

    // Highlight user coordinates
    for (const QString &coord : m_docState->userCoordinates()) {
        highlightWord(text, coord, m_coordNameFormat);
    }

    // Highlight user nodes (but not coordinate names to avoid double highlight)
    for (const QString &node : m_docState->userNodes()) {
        if (!m_docState->userCoordinates().contains(node))
            highlightWord(text, node, m_nodeNameFormat);
    }

        // Highlight foreach variables
        for (const QString &var : m_docState->foreachVars()) {
            static const QRegularExpression varRe(
                QStringLiteral("\\\\[a-zA-Z]+"));
            // Only highlight variables preceded by backslash
            int searchPos = 0;
            while (searchPos < text.length()) {
                int idx = text.indexOf("\\" + var, searchPos);
                if (idx < 0) break;
                int end = idx + 1 + var.length();
                // Check word boundary
                if (end >= text.length() || !text.at(end).isLetter()) {
                    if (format(idx).foreground() != m_commentFormat.foreground())
                        setFormat(idx, end - idx, m_foreachVarFormat);
                }
                searchPos = end;
            }
        }

    // Highlight user commands
    for (const QString &cmd : m_docState->userCommands()) {
        highlightWord(text, cmd, m_userCmdFormat);
    }

    // Highlight \\def\\ and similar
    static const QRegularExpression defCmdRe(
        QStringLiteral("\\\\(?:newcommand|renewcommand|providecommand|def)\\*?\\s*\\{?(\\\\)[a-zA-Z@]+"));
    QRegularExpressionMatchIterator it = defCmdRe.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        if (m.capturedLength() > 2)
            setFormat(m.capturedStart(), m.capturedLength(), m_userCmdFormat);
    }
}

void TikzHighlighter::highlightWord(const QString &text, const QString &word,
                                      const QTextCharFormat &fmt)
{
    if (word.length() < 2) return;
    int searchPos = 0;
    while (searchPos < text.length()) {
        int idx = text.indexOf(word, searchPos);
        if (idx < 0) break;
        // Check word boundary
        bool leftOk = (idx == 0 || !text.at(idx - 1).isLetterOrNumber());
        bool rightOk = (idx + word.length() >= text.length()
                        || !text.at(idx + word.length()).isLetterOrNumber());
        if (leftOk && rightOk) {
            if (format(idx).foreground() != m_commentFormat.foreground())
                setFormat(idx, word.length(), fmt);
        }
        searchPos = idx + 1;
    }
}

void TikzHighlighter::applyKeyValueHighlight(const QString &text)
{
    // Highlight key=value: key in keyFormat, value stays in optionFormat.
    // Only apply inside option brackets ([...]) and at brace depth 0, so a '='
    // nested inside a value's braces (e.g. [name/.style={a=b}]) is not mistaken
    // for a new key/value separator.
    int bracketDepth = 0;
    int braceDepth = 0;
    for (int i = 0; i < text.length(); i++) {
        QChar ch = text.at(i);
        if (ch == '[') bracketDepth++;
        else if (ch == ']') { if (bracketDepth > 0) bracketDepth--; }
        else if (ch == '{') braceDepth++;
        else if (ch == '}') { if (braceDepth > 0) braceDepth--; }
        else if (bracketDepth > 0 && braceDepth == 0 && ch == '=') {
            // Find key start
            int ks = i - 1;
            while (ks >= 0 && (text.at(ks).isLetterOrNumber()
                   || text.at(ks) == ' ' || text.at(ks) == '_'))
                ks--;
            ks++;
            if (ks < i) {
                setFormat(ks, i - ks, m_keyFormat);
            }
        }
    }
}
