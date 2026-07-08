#pragma once
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>
#include <QSet>
#include <QHash>

class TikzDocumentState;

class TikzHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit TikzHighlighter(QTextDocument *parent = nullptr);

    void setDocumentState(TikzDocumentState *state);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
        int priority = 0;
    };

    void applyRules(const QString &text);
    void applyUserHighlights(const QString &text);
    void applyKeyValueHighlight(const QString &text);
    // Colors option brackets ([...]) as m_optionFormat, honoring nesting and
    // spanning multiple lines via the InBracket block state. Returns true if the
    // block ends while still inside an unclosed bracket.
    bool applyOptionBrackets(const QString &text, bool startedInBracket);
    void highlightWord(const QString &text, const QString &word,
                       const QTextCharFormat &fmt);

    QVector<Rule> m_rules;

    QTextCharFormat m_commentFormat;
    QTextCharFormat m_commandFormat;
    QTextCharFormat m_envFormat;
    QTextCharFormat m_mathFormat;
    QTextCharFormat m_braceFormat;
    QTextCharFormat m_coordFormat;
    QTextCharFormat m_optionFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_paramFormat;

    // New formats
    QTextCharFormat m_userStyleFormat;
    QTextCharFormat m_nodeNameFormat;
    QTextCharFormat m_coordNameFormat;
    QTextCharFormat m_foreachVarFormat;
    QTextCharFormat m_userCmdFormat;
    QTextCharFormat m_keyFormat;
    QTextCharFormat m_pgfPathFormat;
    QTextCharFormat m_handlerFormat;
    QTextCharFormat m_libFormat;

    TikzDocumentState *m_docState = nullptr;

    enum BlockState { Normal = 0, InComment = 1, InBracket = 2 };
};
