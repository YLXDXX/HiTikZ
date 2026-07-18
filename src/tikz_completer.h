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
        TkzCtxEnd,
        TkzCtxBrk,
        TkzCtxDot,
        TkzCtxEq,
        TkzCtxAt,
        TkzCtxLib,
        TkzCtxWord,
        TkzCtxCoord,
        TkzCtxUserCmd,
        TkzCtxPathWord,
        TkzCtxCoordSysKey
    };

    explicit TikzCompleter(QPlainTextEdit *editor, QObject *parent = nullptr);

    bool isPopupVisible() const;
    void tryComplete();
    // Manual invocation (Ctrl+Space): forces the popup to appear even when the
    // current context has an empty prefix (e.g. an empty '[' or '(' group),
    // which the automatic path suppresses to avoid noise.
    void tryCompleteManual();
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

    // Option keys of a coordinate system (offered after "(name cs:"), e.g.
    // {angle,radius,z} for "xyz cylindrical". Library-gated. Exposed for testing.
    QStringList coordSysKeysForName(const QString &sysName) const;

    // Returns the current completion-candidate list for a context model.
    // Exposed for testing (e.g. verifying coord model includes intersection-N).
    QStringList modelWordsForContext(Context ctx) const;

    // Rebuilds the user-defined completion models (coords/nodes/commands/...)
    // from the current document state. Exposed for testing.
    void updateUserModels();

    // Rebuilds the coordinate-system key model for the given system name (the
    // keys offered after "(name cs:"). Exposed for testing.
    void updateCoordSysKeyModel(const QString &sysName);

    // Extracts the option key governing the value being typed at the cursor for
    // '=' value completion. Correctly ignores '=', ',' and '[' that are nested
    // inside a value's braces (e.g. [a={x,y}, fill=] -> "fill"). Exposed for
    // testing.
    static QString eqKeyName(const QString &textBefore);

    // Index of the innermost '(' still open at the cursor, or -1. Brace-aware
    // and paren-depth-aware, so closed pairs nested in a value do not hide the
    // governing paren (e.g. "(intersection cs:first line={(A)--(B)}, |" must
    // yield the "(intersection..." paren, not fail on the closed "(B)").
    // Exposed for testing.
    static int governingOpenParenIndex(const QString &textBefore);

    // Index of the last ',' at brace-depth 0 in `segment`, or -1 (commas inside
    // {...} values do not count). Exposed for testing.
    static int lastTopLevelCommaIndex(const QString &segment);

    // Returns the path/node command name governing the option bracket at the
    // cursor: the last \draw/\node/\path/... on the line before `cursorCol`
    // (e.g. "node" for "\draw (0,0) -- \node[|"). Exposed for testing.
    static QString commandForOptionContext(const QString &lineBefore, int cursorCol);

    void setDocumentState(TikzDocumentState *state);

private:
    void initCompleters();
    void doComplete(bool manual);
    void updateBrkModel();
    void updateDotModel();
    void updateEndModel();
    void updateEqModel(const QString &keyName);
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
