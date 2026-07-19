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
    connect(this, &CodeEditor::selectionChanged,
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
    // Manual completion trigger: Ctrl+Space forces the completion popup even in
    // contexts that are otherwise silent until the user types (e.g. an empty
    // '[' or '(' group).
    if (m_completer && event->key() == Qt::Key_Space
        && (event->modifiers() & Qt::ControlModifier)
        && !(event->modifiers() & (Qt::AltModifier | Qt::ShiftModifier))) {
        m_completer->tryCompleteManual();
        return;
    }

    bool completionHandled = false;

    if (m_completer) {
        completionHandled = m_completer->handleCompletionKey(event);
    }

    if (completionHandled) {
        return;
    }

    // ── Tab / Shift+Tab block (de)indentation ──
    // With a selection spanning one or more lines, Tab indents every touched
    // line by one level (4 spaces) and Shift+Tab removes one level. Shift+Tab
    // with no selection dedents the current line. A plain Tab with no selection
    // falls through to the default (insert spaces) behaviour below.
    if ((event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab)
        && !(event->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {
        const bool dedent = (event->key() == Qt::Key_Backtab)
                            || (event->modifiers() & Qt::ShiftModifier);
        QTextCursor cursor = textCursor();
        const bool hasSel = cursor.hasSelection();

        if (hasSel || dedent) {
            static const QString kIndentUnit = QStringLiteral("    ");
            const int indentWidth = kIndentUnit.length();

            int selStart = cursor.selectionStart();
            int selEnd = cursor.selectionEnd();

            QTextCursor c(document());
            c.setPosition(selStart);
            const int firstBlock = c.blockNumber();
            c.setPosition(selEnd);
            // A selection ending exactly at a line start does not include that
            // trailing (empty) line.
            int lastBlock = c.blockNumber();
            if (c.positionInBlock() == 0 && lastBlock > firstBlock)
                lastBlock--;

            cursor.beginEditBlock();
            for (int b = firstBlock; b <= lastBlock; ++b) {
                QTextBlock block = document()->findBlockByNumber(b);
                if (!block.isValid()) continue;
                QTextCursor lineCur(block);
                lineCur.movePosition(QTextCursor::StartOfBlock);
                const QString lineText = block.text();
                if (dedent) {
                    // Remove up to one indent level worth of leading whitespace.
                    int remove = 0;
                    while (remove < indentWidth && remove < lineText.length()) {
                        const QChar ch = lineText.at(remove);
                        if (ch == QLatin1Char('\t')) { remove++; break; }
                        if (ch == QLatin1Char(' ')) { remove++; continue; }
                        break;
                    }
                    if (remove > 0) {
                        lineCur.movePosition(QTextCursor::Right,
                                             QTextCursor::KeepAnchor, remove);
                        lineCur.removeSelectedText();
                    }
                } else {
                    // Do not indent completely empty lines.
                    if (!lineText.isEmpty())
                        lineCur.insertText(kIndentUnit);
                }
            }
            cursor.endEditBlock();

            // Re-select the affected line range so repeated Tab presses keep
            // operating on the same block — but only when the user started with
            // a selection. A bare Shift+Tab on one line just keeps the caret.
            if (hasSel) {
                QTextBlock fb = document()->findBlockByNumber(firstBlock);
                QTextBlock lb = document()->findBlockByNumber(lastBlock);
                if (fb.isValid() && lb.isValid()) {
                    QTextCursor sel(document());
                    sel.setPosition(fb.position());
                    sel.setPosition(lb.position() + lb.length() - 1,
                                    QTextCursor::KeepAnchor);
                    setTextCursor(sel);
                }
            }
            return;
        }
        // No selection + plain Tab: fall through to default indentation below.
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

        // Special case: cursor sits directly between '{' and '}' (i.e. "{|}")
        // and the '{' is at the start of the line (only indentation precedes
        // it). Split the pair over three lines, placing the closing brace on
        // its own line aligned with the opening brace's indentation and leaving
        // the cursor on an indented middle line:
        //     {
        //         |
        //     }
        {
            QTextCursor probe = textCursor();
            if (!probe.hasSelection() && currentLine.trimmed() == QLatin1String("{")) {
                QTextDocument *d = document();
                int p = probe.position();
                QChar before = p >= 1 ? d->characterAt(p - 1) : QChar();
                QChar after  = d->characterAt(p);
                if (before == QLatin1Char('{') && after == QLatin1Char('}')) {
                    QTextCursor edit = textCursor();
                    edit.beginEditBlock();
                    edit.insertText(QLatin1String("\n") + indent
                                    + QLatin1String("    ")
                                    + QLatin1String("\n") + indent);
                    edit.endEditBlock();
                    // Position the cursor on the middle (indented) line.
                    int mid = p + 1 + indent.length() + 4;
                    edit.setPosition(mid);
                    setTextCursor(edit);
                    return;
                }
            }
        }

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
                    // After wrapping, re-select the inner text so the user
                    // can continue typing or add another bracket.
                    int selStart = cursor.selectionStart();
                    int selEnd = cursor.selectionEnd();
                    cursor.beginEditBlock();
                    cursor.setPosition(selStart);
                    cursor.insertText(QString(open));
                    cursor.setPosition(selEnd + 1);
                    cursor.insertText(QString(close));
                    cursor.endEditBlock();
                    cursor.setPosition(selStart + 1);
                    cursor.setPosition(selEnd + 1, QTextCursor::KeepAnchor);
                    setTextCursor(cursor);
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
            // Inline math '$': behaves like the brackets above but is its own
            // closer. Wrap a selection as $selection$; otherwise insert a '$$'
            // pair with the cursor between them — except when the next character
            // is already a '$', in which case skip over it (mirrors the
            // closing-bracket skip below) so typing '$' to close a pair doesn't
            // produce '$$'.
            else if (text == QLatin1String("$")) {
                if (cursor.hasSelection()) {
                    int selStart = cursor.selectionStart();
                    int selEnd = cursor.selectionEnd();
                    cursor.beginEditBlock();
                    cursor.setPosition(selStart);
                    cursor.insertText(QStringLiteral("$"));
                    cursor.setPosition(selEnd + 1);
                    cursor.insertText(QStringLiteral("$"));
                    cursor.endEditBlock();
                    cursor.setPosition(selStart + 1);
                    cursor.setPosition(selEnd + 1, QTextCursor::KeepAnchor);
                    setTextCursor(cursor);
                    handled = true;
                } else {
                    // Skip over a following '$' that closes an existing pair.
                    bool skipped = false;
                    if (!cursor.atEnd()) {
                        QTextCursor probe = cursor;
                        probe.movePosition(QTextCursor::Right,
                                           QTextCursor::KeepAnchor, 1);
                        if (probe.selectedText() == QLatin1String("$")) {
                            probe.clearSelection();
                            setTextCursor(probe);
                            handled = true;
                            skipped = true;
                        }
                    }
                    if (!skipped) {
                        cursor.movePosition(QTextCursor::StartOfBlock,
                                            QTextCursor::KeepAnchor);
                        QString lineStart = cursor.selectedText().trimmed();
                        cursor = textCursor();   // restore original position
                        bool inComment = lineStart.startsWith(QLatin1Char('%'));
                        if (!inComment) {
                            cursor.insertText(QStringLiteral("$$"));
                            cursor.movePosition(QTextCursor::Left,
                                                QTextCursor::MoveAnchor, 1);
                            setTextCursor(cursor);
                            handled = true;
                        }
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
                        // clearSelection leaves the cursor right after the
                        // matched bracket — no extra move needed.
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
            if (!cursor.hasSelection() && cursor.position() >= 1
                && cursor.position() < document()->characterCount() - 1) {
                int pos = cursor.position();
                QChar before = document()->characterAt(pos - 1);
                QChar after  = document()->characterAt(pos);
                bool isPair =
                    (before == QLatin1Char('{') && after == QLatin1Char('}')) ||
                    (before == QLatin1Char('[') && after == QLatin1Char(']')) ||
                    (before == QLatin1Char('(') && after == QLatin1Char(')')) ||
                    (before == QLatin1Char('$') && after == QLatin1Char('$'));
                if (isPair) {
                    cursor.setPosition(pos - 1);
                    cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
                    cursor.removeSelectedText();
                    setTextCursor(cursor);
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

bool CodeEditor::isOpenBracket(QChar ch)
{
    return ch == QLatin1Char('{') || ch == QLatin1Char('[')
        || ch == QLatin1Char('(');
}

bool CodeEditor::isCloseBracket(QChar ch)
{
    return ch == QLatin1Char('}') || ch == QLatin1Char(']')
        || ch == QLatin1Char(')');
}

QChar CodeEditor::closingBracketFor(QChar open)
{
    if (open == QLatin1Char('{')) return QLatin1Char('}');
    if (open == QLatin1Char('[')) return QLatin1Char(']');
    if (open == QLatin1Char('(')) return QLatin1Char(')');
    return QChar();
}

int CodeEditor::findMatchingBracket(int pos, QChar bracket, const QTextDocument *doc) const
{
    const int totalChars = doc->characterCount();
    // QTextDocument::characterCount() includes the implicit paragraph
    // separator at the very end; stay within the actual text range.
    const int lastIdx = totalChars - 1;

    if (isOpenBracket(bracket)) {
        const QChar close = closingBracketFor(bracket);
        int depth = 0;
        for (int i = pos + 1; i < lastIdx; ++i) {
            const QChar ch = doc->characterAt(i);
            if (ch == bracket) {
                depth++;
            } else if (ch == close) {
                if (depth == 0) return i;
                depth--;
            }
        }
    } else if (isCloseBracket(bracket)) {
        QChar open;
        if (bracket == QLatin1Char('}'))      open = QLatin1Char('{');
        else if (bracket == QLatin1Char(']')) open = QLatin1Char('[');
        else                                  open = QLatin1Char('(');

        int depth = 0;
        for (int i = pos - 1; i >= 0; --i) {
            const QChar ch = doc->characterAt(i);
            if (ch == bracket) {
                depth++;
            } else if (ch == open) {
                if (depth == 0) return i;
                depth--;
            }
        }
    }
    return -1;
}

void CodeEditor::performBracketHighlight(QList<QTextEdit::ExtraSelection> &extraSelections)
{
    QTextCursor cursor = textCursor();
    QTextDocument *doc = document();
    const int totalChars = doc->characterCount();
    if (totalChars <= 1) return;

    int bracketPos = -1;
    QChar bracket;

    // Priority 1: user has selected a single bracket character.
    if (cursor.hasSelection()) {
        const QString sel = cursor.selectedText();
        if (sel.length() == 1) {
            const QChar ch = sel.at(0);
            if (isOpenBracket(ch) || isCloseBracket(ch)) {
                bracket = ch;
                bracketPos = cursor.selectionStart();
            }
        }
    }

    // Priority 2: cursor is next to (left or right of) a bracket.
    if (bracketPos < 0) {
        const int pos = cursor.position();

        // Check the character to the right of cursor.
        if (pos < totalChars - 1) {
            const QChar ch = doc->characterAt(pos);
            if (isOpenBracket(ch) || isCloseBracket(ch)) {
                bracket = ch;
                bracketPos = pos;
            }
        }

        // Check the character to the left of cursor.
        if (bracketPos < 0 && pos > 0) {
            const QChar ch = doc->characterAt(pos - 1);
            if (isOpenBracket(ch) || isCloseBracket(ch)) {
                bracket = ch;
                bracketPos = pos - 1;
            }
        }
    }

    if (bracketPos < 0) return;

    const int matchPos = findMatchingBracket(bracketPos, bracket, doc);

    // Highlight the bracket at bracketPos.
    {
        QTextEdit::ExtraSelection sel;
        QTextCursor hc(doc);
        hc.setPosition(bracketPos);
        hc.setPosition(bracketPos + 1, QTextCursor::KeepAnchor);
        if (matchPos >= 0) {
            sel.format.setBackground(QColor(100, 200, 255));
        } else {
            sel.format.setBackground(QColor(255, 100, 100));
        }
        sel.cursor = hc;
        extraSelections.append(sel);
    }

    // Highlight the match.
    if (matchPos >= 0) {
        QTextEdit::ExtraSelection sel;
        QTextCursor mc(doc);
        mc.setPosition(matchPos);
        mc.setPosition(matchPos + 1, QTextCursor::KeepAnchor);
        sel.format.setBackground(QColor(100, 200, 255));
        sel.cursor = mc;
        extraSelections.append(sel);
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

    performBracketHighlight(extraSelections);

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
