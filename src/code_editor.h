#pragma once
#include <QPlainTextEdit>
#include <QWidget>
#include <QTimer>

class LineNumberArea;
class TikzHighlighter;
class TikzCompleter;
class TikzDocumentState;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor() override;

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    // Toggle soft-wrapping of over-long lines. When enabled, wrapped
    // continuation rows are marked in the line-number gutter.
    void setWordWrap(bool wrap);

    TikzCompleter *completer() const;
    TikzDocumentState *documentState() const;

public slots:
    void highlightCurrentLine();
    void refreshParamWords(const QStringList &params);
    void reparseDocumentState();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
    TikzHighlighter *m_highlighter;
    TikzCompleter *m_completer;
    TikzDocumentState *m_docState;
    bool m_focusInProgress = false;
    QTimer *m_highlightDebounceTimer = nullptr;
    QTimer *m_reparseTimer = nullptr;

    void performHighlightCurrentLine();
    void performBracketHighlight(QList<QTextEdit::ExtraSelection> &extraSelections);
    int findMatchingBracket(int pos, QChar bracket, const QTextDocument *doc) const;
    static QChar closingBracketFor(QChar open);
    static bool isOpenBracket(QChar ch);
    static bool isCloseBracket(QChar ch);
};

class LineNumberArea : public QWidget {
    Q_OBJECT
public:
    LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *codeEditor;
};
