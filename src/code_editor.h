#pragma once
#include <QPlainTextEdit>
#include <QWidget>

class LineNumberArea;
class TikzHighlighter;
class TikzCompleter;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    TikzCompleter *completer() const;

public slots:
    void highlightCurrentLine();

    void refreshParamWords(const QStringList &params);

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
    bool m_focusInProgress = false;
    QTimer *m_highlightDebounceTimer = nullptr;

    void performHighlightCurrentLine();
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
