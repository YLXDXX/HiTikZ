#include "code_editor.h"
#include "tikz_highlighter.h"
#include "tikz_completer.h"
#include "tikz_document_state.h"
#include <QPainter>
#include <QTextBlock>
#include <QTextLayout>
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

    m_highlightDebounceTimer = new QTimer(this);
    m_highlightDebounceTimer->setSingleShot(true);
    m_highlightDebounceTimer->setInterval(200);
    connect(m_highlightDebounceTimer, &QTimer::timeout,
            this, &CodeEditor::performHighlightCurrentLine);

    m_docState = new TikzDocumentState();
    m_completer->setDocumentState(m_docState);
    m_highlighter->setDocumentState(m_docState);

    m_reparseTimer = new QTimer(this);
    m_reparseTimer->setSingleShot(true);
    m_reparseTimer->setInterval(300);
    connect(m_reparseTimer, &QTimer::timeout,
            this, &CodeEditor::reparseDocumentState);
    connect(this, &CodeEditor::textChanged,
            this, [this]() { m_reparseTimer->start(); });

    setMouseTracking(true);
}

CodeEditor::~CodeEditor()
{
    delete m_docState;
}

TikzCompleter *CodeEditor::completer() const
{
    return m_completer;
}

TikzDocumentState *CodeEditor::documentState() const
{
    return m_docState;
}

void CodeEditor::reparseDocumentState()
{
    if (m_docState) {
        m_docState->reparse(document());
        if (m_highlighter)
            m_highlighter->rehighlight();
    }
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
    // Ensure the wrapped-continuation marker fits even for single-digit files.
    space = qMax(space, 3 + fontMetrics().horizontalAdvance(QStringLiteral("\u21B3")) + 4);
    return space;
}

void CodeEditor::setWordWrap(bool wrap)
{
    setLineWrapMode(wrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    // The logical->visual line mapping changed; refresh the gutter.
    if (lineNumberArea)
        lineNumberArea->update();
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

    // ── Auto-pair brackets / braces / parentheses ──
    {
        const QString &text = event->text();
        auto mods = event->modifiers();
        if (!text.isEmpty() && !(mods & (Qt::ControlModifier | Qt::AltModifier))) {
            QTextCursor cursor = textCursor();
            bool handled = false;

            // Opening brackets: insert pair and place cursor between them.
            if (text == QLatin1String("{") || text == QLatin1String("[")
                || text == QLatin1String("(")) {
                QChar open = text.at(0);
                QChar close;
                if (open == QLatin1Char('{'))      close = QLatin1Char('}');
                else if (open == QLatin1Char('[')) close = QLatin1Char(']');
                else                               close = QLatin1Char(')');

                if (cursor.hasSelection()) {
                    // Wrap the selected text:  {selection}  [selection]  etc.
                    int selStart = cursor.selectionStart();
                    int selEnd = cursor.selectionEnd();
                    cursor.setPosition(selStart);
                    cursor.insertText(QString(open));
                    cursor.setPosition(selEnd + 1);
                    cursor.insertText(QString(close));
                    handled = true;
                } else {
                    // Check whether we are on a comment line — don't
                    // auto-pair inside comments.
                    cursor.movePosition(QTextCursor::StartOfBlock,
                                        QTextCursor::KeepAnchor);
                    QString lineStart = cursor.selectedText().trimmed();
                    cursor = textCursor();   // restore original position
                    bool inComment = lineStart.startsWith(QLatin1Char('%'));

                    if (!inComment) {
                        cursor.insertText(QString(open) + QString(close));
                        cursor.movePosition(QTextCursor::Left,
                                            QTextCursor::MoveAnchor, 1);
                        setTextCursor(cursor);
                        handled = true;
                    }
                }
            }
            // Closing brackets: skip over if the next character is the same.
            else if (text == QLatin1String("}") || text == QLatin1String("]")
                     || text == QLatin1String(")")) {
                QChar close = text.at(0);
                if (!cursor.hasSelection() && !cursor.atEnd()) {
                    cursor.movePosition(QTextCursor::Right,
                                        QTextCursor::KeepAnchor, 1);
                    if (cursor.selectedText().at(0) == close) {
                        cursor.clearSelection();
                        cursor.movePosition(QTextCursor::Right,
                                            QTextCursor::MoveAnchor, 1);
                        setTextCursor(cursor);
                        handled = true;
                    }
                }
            }

            if (handled) {
                if (m_completer)
                    QTimer::singleShot(0, this,
                                       [this]() { m_completer->tryComplete(); });
                return;
            }
        }

        // Backspace inside an empty bracket pair: delete both characters.
        if (event->key() == Qt::Key_Backspace && !(mods & Qt::ShiftModifier)) {
            QTextCursor cursor = textCursor();
            if (!cursor.hasSelection() && cursor.position() >= 2
                && cursor.position() < document()->characterCount() - 1) {
                int pos = cursor.position();
                QChar before = document()->characterAt(pos - 1);
                QChar after  = document()->characterAt(pos);
                bool isPair =
                    (before == QLatin1Char('{') && after == QLatin1Char('}')) ||
                    (before == QLatin1Char('[') && after == QLatin1Char(']')) ||
                    (before == QLatin1Char('(') && after == QLatin1Char(')'));
                if (isPair) {
                    cursor.setPosition(pos);
                    cursor.setPosition(pos + 2, QTextCursor::KeepAnchor);
                    cursor.removeSelectedText();
                    if (m_completer)
                        QTimer::singleShot(0, this,
                                           [this]() { m_completer->tryComplete(); });
                    return;
                }
            }
        }
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
    if (m_highlightDebounceTimer) {
        m_highlightDebounceTimer->start();
    }
}

void CodeEditor::performHighlightCurrentLine()
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
            findCursor = doc->find(word, findCursor, QTextDocument::FindCaseSensitively
                                    | QTextDocument::FindWholeWords);
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
    const int areaWidth = lineNumberArea->width();
    const int fh = fontMetrics().height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QTextLayout *layout = block.layout();
            const int lineCount = layout ? layout->lineCount() : 1;
            for (int i = 0; i < lineCount; ++i) {
                int lineTop = top;
                if (layout && i < layout->lineCount())
                    lineTop = top + qRound(layout->lineAt(i).rect().top());

                if (i == 0) {
                    // Real (logical) line: show its number.
                    painter.setPen(Qt::darkGray);
                    painter.drawText(0, lineTop, areaWidth, fh, Qt::AlignRight,
                                     QString::number(blockNumber + 1));
                } else {
                    // Soft-wrapped continuation of the same logical line:
                    // show a marker instead of a number.
                    painter.setPen(QColor(120, 140, 200));
                    painter.drawText(0, lineTop, areaWidth, fh, Qt::AlignRight,
                                     QStringLiteral("\u21B3"));
                }
            }
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
