#pragma once
#include <QObject>
#include <QCompleter>
#include <QPlainTextEdit>
#include <QHash>
#include <QStringListModel>

class TikzDocumentState;

class TikzCompleter : public QObject {
    Q_OBJECT
public:
    enum Context {
        TkzCtxNone,
        TkzCtxCmd,
        TkzCtxBeg,
        TkzCtxBrk,
        TkzCtxDot,
        TkzCtxEq,
        TkzCtxAt,
        TkzCtxLib,
        TkzCtxWord,
        TkzCtxCoord,
        TkzCtxUserCmd,
        TkzCtxPathWord
    };

    explicit TikzCompleter(QPlainTextEdit *editor, QObject *parent = nullptr);

    bool isPopupVisible() const;
    void tryComplete();
    bool handleCompletionKey(QKeyEvent *event);

    Context detectContext(const QString &textBeforeCursor) const;

    // Text from the cursor back to the nearest unclosed '[' or '{' (or up to a
    // bounded look-back), used as the context for completion. Unlike the current
    // line alone, this lets option/brace contexts work when written across
    // multiple lines. Exposed for testing.
    QString textBeforeForContext() const;

    void refreshParamWords(const QStringList &params);

    void setModelForContext(Context ctx, const QStringList &words);

    // Candidate values offered after "key=" in an options bracket.
    // Exposed for testing.
    QStringList eqCandidatesForKey(const QString &keyName) const;

    // Extracts the option key governing the value being typed at the cursor for
    // '=' value completion. Correctly ignores '=', ',' and '[' that are nested
    // inside a value's braces (e.g. [a={x,y}, fill=] -> "fill"). Exposed for
    // testing.
    static QString eqKeyName(const QString &textBefore);

    // Returns the path/node command name governing the option bracket at the
    // cursor: the last \draw/\node/\path/... on the line before `cursorCol`
    // (e.g. "node" for "\draw (0,0) -- \node[|"). Exposed for testing.
    static QString commandForOptionContext(const QString &lineBefore, int cursorCol);

    void setDocumentState(TikzDocumentState *state);

private:
    void initCompleters();
    void updateBrkModel();
    void updateDotModel();
    void updateEqModel(const QString &keyName);
    void updateUserModels();
    QStringList buildBrkCandidates(Context ctx);
    // Index of the '=' governing the value at the cursor (brace-depth 0), or -1.
    static int governingEqIndex(const QString &textBefore);

    QPlainTextEdit *m_editor;
    QHash<Context, QCompleter *> m_completers;
    QHash<Context, QStringListModel *> m_models;
    Context m_activeContext = TkzCtxNone;
    mutable QString m_lastTextBeforeCursor;
    mutable Context m_lastContext = TkzCtxNone;
    TikzDocumentState *m_docState = nullptr;
};
