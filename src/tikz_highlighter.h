#pragma once
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>

class TikzHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit TikzHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat format;
        int priority = 0;
    };

    void applyRules(const QString &text);

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

    enum BlockState { Normal = 0, InComment = 1 };
};

