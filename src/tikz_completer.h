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
        TkzCtxUserCmd
    };

    explicit TikzCompleter(QPlainTextEdit *editor, QObject *parent = nullptr);

    bool isPopupVisible() const;
    void tryComplete();
    bool handleCompletionKey(QKeyEvent *event);

    Context detectContext(const QString &textBeforeCursor) const;

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

    void setDocumentState(TikzDocumentState *state);

private:
    void initCompleters();
    void updateBrkModel();
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
