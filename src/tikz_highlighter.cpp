#include "tikz_highlighter.h"

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

    m_rules = {
        {QRegularExpression(QStringLiteral("%[^\n]*")), m_commentFormat, 100},
        {QRegularExpression(QStringLiteral("\"[^\"]*\"")), m_stringFormat, 1},
        {QRegularExpression(QStringLiteral("@@[a-zA-Z_][a-zA-Z0-9_]*@@")), m_paramFormat, 2},
        {QRegularExpression(QStringLiteral("\\\\begin\\{[^}]*\\}|\\\\end\\{[^}]*\\}")), m_envFormat, 3},
        {QRegularExpression(QStringLiteral("\\\\[a-zA-Z@][a-zA-Z@]*(\\s*\\*)?")), m_commandFormat, 4},
        {QRegularExpression(QStringLiteral("\\$[^\\$]*\\$")), m_mathFormat, 5},
        {QRegularExpression(QStringLiteral("\\\\\\(.*?\\\\\\)")), m_mathFormat, 6},
        {QRegularExpression(QString::fromUtf8(R"(\\\[.*?\\\])")), m_mathFormat, 7},
        {QRegularExpression(QStringLiteral("\\{[^}]*\\}|\\}")), m_braceFormat, 8},
        {QRegularExpression(QStringLiteral("\\(-?[\\d.]+\\.?\\s*,\\s*-?[\\d.]+\\s*\\)")), m_coordFormat, 9},
        {QRegularExpression(QStringLiteral("\\[[^\\]]*\\]")), m_optionFormat, 10},
        {QRegularExpression(QStringLiteral("\\b\\d+\\.?\\d*(pt|cm|mm|in|ex|em|bp|dd|pc|cc|sp)?\\b")), m_numberFormat, 11},
    };

    std::sort(m_rules.begin(), m_rules.end(),
              [](const Rule &a, const Rule &b) { return a.priority < b.priority; });
}

void TikzHighlighter::highlightBlock(const QString &text)
{
    int prevState = previousBlockState();
    if (prevState == InComment) {
        setFormat(0, text.length(), m_commentFormat);
        static const QRegularExpression endCommentRe(
            QString::fromUtf8(R"(\\end\{comment\})"));
        QRegularExpressionMatch endMatch = endCommentRe.match(text);
        bool hasEnd = endMatch.hasMatch();
        setCurrentBlockState(hasEnd ? Normal : InComment);
        if (!hasEnd) return;
        int afterEnd = endMatch.capturedStart() + endMatch.capturedLength();
        if (afterEnd < text.length())
            applyRules(text.mid(afterEnd));
        return;
    }

    if (text.trimmed().startsWith('%')) {
        setFormat(0, text.length(), m_commentFormat);
        setCurrentBlockState(Normal);
        return;
    }

    applyRules(text);

    static const QRegularExpression beginCommentRe(
        QString::fromUtf8(R"(\\begin\{comment\})"));
    QRegularExpressionMatchIterator it = beginCommentRe.globalMatch(text);
    bool endsInComment = false;
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        static const QRegularExpression endCommentRe(
            QString::fromUtf8(R"(\\end\{comment\})"));
        QString afterBegin = text.mid(m.capturedEnd());
        if (!endCommentRe.match(afterBegin).hasMatch()) {
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
