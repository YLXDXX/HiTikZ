#include "code_editor.h"
#include "tikz_highlighter.h"
#include "tikz_completer.h"
#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QTimer>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_highlighter = new TikzHighlighter(document());

    m_completer = new TikzCompleter(this, this);

    setMouseTracking(true);
}

TikzCompleter *CodeEditor::completer() const
{
    return m_completer;
}

void CodeEditor::refreshParamWords(const QStringList &params)
{
    if (m_completer)
        m_completer->refreshParamWords(params);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    bool completionHandled = false;

    if (m_completer) {
        completionHandled = m_completer->handleCompletionKey(event);
    }

    if (completionHandled) {
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        QString currentLine = cursor.selectedText();

        int indentEnd = 0;
        while (indentEnd < currentLine.length()
               && (currentLine.at(indentEnd) == ' ' || currentLine.at(indentEnd) == '\t')) {
            indentEnd++;
        }
        QString indent = currentLine.left(indentEnd);

        QString trimmed = currentLine.trimmed();
        bool extraIndent = false;
        if (trimmed.endsWith('{')
            || (trimmed.startsWith("\\begin") && !trimmed.contains("\\end"))) {
            extraIndent = true;
        }

        QPlainTextEdit::keyPressEvent(event);

        cursor = textCursor();
        cursor.insertText(indent);
        if (extraIndent)
            cursor.insertText("    ");
        setTextCursor(cursor);

        if (m_completer) {
            QTimer::singleShot(0, this, [this]() { m_completer->tryComplete(); });
        }
        return;
    }

    QPlainTextEdit::keyPressEvent(event);

    if (m_completer && !event->text().isEmpty()) {
        QTimer::singleShot(0, this, [this]() { m_completer->tryComplete(); });
    }
}

void CodeEditor::focusInEvent(QFocusEvent *event)
{
    m_focusInProgress = true;
    QPlainTextEdit::focusInEvent(event);
    m_focusInProgress = false;
    highlightCurrentLine();
}

void CodeEditor::focusOutEvent(QFocusEvent *event)
{
    QPlainTextEdit::focusOutEvent(event);
    highlightCurrentLine();
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly() && !m_focusInProgress) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(180);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    QTextCursor cursor = textCursor();
    QString word;
    if (cursor.hasSelection()) {
        word = cursor.selectedText().trimmed();
    } else {
        cursor.clearSelection();
        cursor.select(QTextCursor::WordUnderCursor);
        word = cursor.selectedText().trimmed();
    }

    if (word.length() > 1 && !word.contains(' ') && !word.contains('\n')
        && !word.contains('\t')) {
        QColor wordColor(255, 230, 180);

        QTextDocument *doc = document();
        QTextCursor findCursor(doc);

        while (!findCursor.isNull() && !findCursor.atEnd()) {
            findCursor = doc->find(word, findCursor, QTextDocument::FindCaseSensitively);
            if (!findCursor.isNull()) {
                QTextEdit::ExtraSelection wordSel;
                wordSel.format.setBackground(wordColor);
                wordSel.cursor = findCursor;
                extraSelections.append(wordSel);
            }
        }
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(120));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(0, top, lineNumberArea->width(),
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor), codeEditor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}
