#pragma once
#include <QPlainTextEdit>
#include <QWidget>

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget *parent = nullptr) : QPlainTextEdit(parent) {}
    void lineNumberPaintEvent(QPaintEvent *event) { (void)event; }
    int lineNumberAreaWidth() { return 0; }
protected:
    void resizeEvent(QResizeEvent *event) override { QPlainTextEdit::resizeEvent(event); }
private:
    QWidget *lineNumberArea = nullptr;
};
