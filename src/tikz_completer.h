#pragma once
#include <QObject>
#include <QCompleter>
#include <QPlainTextEdit>
#include <QHash>
#include <QStringListModel>

class TikzCompleter : public QObject {
    Q_OBJECT
public:
    enum Context {
        TkzCtxNone,
        TkzCtxCmd,          // after backslash
        TkzCtxBeg,          // after begin
        TkzCtxBrk,          // inside brackets
        TkzCtxDot,          // after dot
        TkzCtxEq,           // after equals
        TkzCtxAt,           // after double at
        TkzCtxLib,          // after uselibrary
        TkzCtxWord          // typing any known word
    };

    explicit TikzCompleter(QPlainTextEdit *editor, QObject *parent = nullptr);

    bool isPopupVisible() const;
    void tryComplete();
    bool handleCompletionKey(QKeyEvent *event);

    Context detectContext(const QString &textBeforeCursor) const;

    void refreshParamWords(const QStringList &params);

    void setModelForContext(Context ctx, const QStringList &words);

private:
    void initCompleters();
    void showCompleter(Context ctx, const QPoint &pos, const QString &prefix);

    QPlainTextEdit *m_editor;
    QHash<Context, QCompleter *> m_completers;
    QHash<Context, QStringListModel *> m_models;
    Context m_activeContext = TkzCtxNone;
};

