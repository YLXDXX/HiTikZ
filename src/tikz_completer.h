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

    void setDocumentState(TikzDocumentState *state);

private:
    void initCompleters();
    void updateBrkModel();
    void updateEqModel(const QString &keyName);
    void updateUserModels();
    QStringList buildBrkCandidates(Context ctx);

    QPlainTextEdit *m_editor;
    QHash<Context, QCompleter *> m_completers;
    QHash<Context, QStringListModel *> m_models;
    Context m_activeContext = TkzCtxNone;
    mutable QString m_lastTextBeforeCursor;
    mutable Context m_lastContext = TkzCtxNone;
    TikzDocumentState *m_docState = nullptr;
};
