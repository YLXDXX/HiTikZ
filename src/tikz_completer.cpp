#include "tikz_completer.h"
#include "tikz_words.h"
#include "tikz_keywords.h"
#include "tikz_document_state.h"
#include <QAbstractItemView>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTextCursor>
#include <QTextDocument>
#include <QRegularExpression>
#include <QWindow>

TikzCompleter::TikzCompleter(QPlainTextEdit *editor, QObject *parent)
    : QObject(parent), m_editor(editor)
{
    initCompleters();
}

void TikzCompleter::setDocumentState(TikzDocumentState *state)
{
    m_docState = state;
}

bool TikzCompleter::isPopupVisible() const
{
    if (m_activeContext == TkzCtxNone) return false;
    if (!m_completers.contains(m_activeContext)) return false;
    return m_completers[m_activeContext]->popup()->isVisible();
}

int TikzCompleter::governingEqIndex(const QString &textBefore)
{
    // Scan backward for the '=' at brace-depth 0 relative to the cursor, so an
    // '=' nested inside a value's braces (e.g. .code={\pgfkeys{x=1}}) is skipped.
    int bDepth = 0;
    for (int i = textBefore.length() - 1; i >= 0; --i) {
        const QChar ch = textBefore.at(i);
        if (ch == '}') bDepth++;
        else if (ch == '{') { if (bDepth > 0) bDepth--; }
        else if (ch == '=' && bDepth == 0) return i;
    }
    return -1;
}

QString TikzCompleter::eqKeyName(const QString &textBefore)
{
    const int eqIdx = governingEqIndex(textBefore);
    if (eqIdx < 0) return QString();

    const QString beforeKey = textBefore.left(eqIdx);
    // Walk back to the token start: the previous ',' or '[' at bracket-depth 0,
    // or the enclosing '{' we step out of — all while ignoring content nested
    // inside braces so commas/brackets inside a value don't split the key.
    int keyStart = -1;
    int kBrace = 0;
    int kBracket = 0;
    for (int i = beforeKey.length() - 1; i >= 0; --i) {
        const QChar ch = beforeKey.at(i);
        if (ch == '}') { kBrace++; continue; }
        if (ch == '{') {
            if (kBrace > 0) { kBrace--; continue; }
            keyStart = i; break;   // stepped out of the enclosing group
        }
        if (kBrace != 0) continue;
        if (ch == ']') { kBracket++; }
        else if (ch == '[') {
            if (kBracket == 0) { keyStart = i; break; }
            kBracket--;
        } else if (ch == ',' && kBracket == 0) {
            keyStart = i; break;
        }
    }
    return beforeKey.mid(keyStart + 1).trimmed();
}

QString TikzCompleter::textBeforeForContext() const
{
    QTextCursor cur = m_editor->textCursor();
    const int cursorAbsPos = cur.position();
    QTextDocument *doc = m_editor->document();

    // Walk backwards to the nearest unclosed '[' or '{' so that option ([...])
    // and argument ({...}) contexts still resolve when they span several lines.
    // Bounded look-back keeps this cheap on very large documents.
    int scan = cursorAbsPos;
    const int limitPos = qMax(0, cursorAbsPos - 5000);
    int bracketDepth = 0;
    int braceDepth = 0;
    while (scan > limitPos) {
        const QChar ch = doc->characterAt(scan - 1);
        if (ch == QLatin1Char(']')) {
            bracketDepth++;
        } else if (ch == QLatin1Char('[')) {
            if (bracketDepth == 0) { scan--; break; } // include the unclosed '['
            bracketDepth--;
        } else if (ch == QLatin1Char('}')) {
            braceDepth++;
        } else if (ch == QLatin1Char('{')) {
            if (braceDepth == 0) {
                scan--; // include the unclosed '{'
                // Also pull in a preceding "\command" (e.g. \usetikzlibrary or
                // \begin) so brace-based contexts that key off the command name
                // still resolve — otherwise only "{...}" survives and the
                // context is misdetected as a generic word.
                int p = scan; // index of the '{'
                while (p > limitPos && doc->characterAt(p - 1).isSpace())
                    p--;
                int wordEnd = p;
                while (p > limitPos && doc->characterAt(p - 1).isLetter())
                    p--;
                if (p < wordEnd && p > limitPos
                    && doc->characterAt(p - 1) == QLatin1Char('\\')) {
                    scan = p - 1; // start at the backslash of the command
                }
                break;
            }
            braceDepth--;
        }
        scan--;
    }

    QTextCursor sel = m_editor->textCursor();
    sel.setPosition(scan);
    sel.setPosition(cursorAbsPos, QTextCursor::KeepAnchor);
    QString textBefore = sel.selectedText();
    // QTextCursor uses U+2029 as the line separator; normalise to '\n'.
    textBefore.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    return textBefore;
}

void TikzCompleter::tryComplete()
{
    QString textBefore = textBeforeForContext();

    // Update user models from document state
    updateUserModels();

    Context ctx;
    if (textBefore == m_lastTextBeforeCursor) {
        ctx = m_lastContext;
    } else {
        ctx = detectContext(textBefore);
        m_lastTextBeforeCursor = textBefore;
        m_lastContext = ctx;
    }

    if (ctx == TkzCtxNone) {
        for (auto *comp : m_completers)
            comp->popup()->hide();
        m_activeContext = TkzCtxNone;
        return;
    }

    QString prefix;
    switch (ctx) {
    case TkzCtxCmd: {
        int lastSlash = textBefore.lastIndexOf('\\');
        if (lastSlash >= 0) prefix = textBefore.mid(lastSlash);
        break;
    }
    case TkzCtxBeg:
    case TkzCtxEnd: {
        int lastBrace = textBefore.lastIndexOf('{');
        if (lastBrace >= 0) {
            int closePos = textBefore.indexOf('}', lastBrace);
            prefix = (closePos < 0) ? textBefore.mid(lastBrace + 1).trimmed()
                                    : textBefore.mid(lastBrace + 1, closePos - lastBrace - 1).trimmed();
        }
        break;
    }
    case TkzCtxLib: {
        int lastBrace = textBefore.lastIndexOf('{');
        if (lastBrace >= 0) {
            int closePos = textBefore.indexOf('}', lastBrace);
            QString inner = (closePos < 0) ? textBefore.mid(lastBrace + 1)
                                           : textBefore.mid(lastBrace + 1, closePos - lastBrace - 1);
            int lastComma = inner.lastIndexOf(',');
            prefix = (lastComma >= 0) ? inner.mid(lastComma + 1).trimmed() : inner.trimmed();
        }
        break;
    }
    case TkzCtxDot: {
        int lastDot = textBefore.lastIndexOf('.');
        if (lastDot >= 0) prefix = textBefore.mid(lastDot + 1);
        break;
    }
    case TkzCtxBrk: {
        int lastBracket = textBefore.lastIndexOf('[');
        int lastComma = textBefore.lastIndexOf(',');
        if (lastBracket >= 0) {
            int start = qMax(lastBracket, lastComma) + 1;
            prefix = textBefore.mid(start).trimmed();
        }
        break;
    }
    case TkzCtxEq: {
        int eqIdx = governingEqIndex(textBefore);
        if (eqIdx >= 0) {
            updateEqModel(eqKeyName(textBefore));
            prefix = textBefore.mid(eqIdx + 1).trimmed();
        }
        break;
    }
    case TkzCtxAt: {
        int atIdx = textBefore.lastIndexOf('@');
        if (atIdx > 0 && textBefore.at(atIdx - 1) == '@') atIdx--;
        if (atIdx >= 0) prefix = textBefore.mid(atIdx);
        break;
    }
    case TkzCtxWord:
    case TkzCtxPathWord: {
        int j = textBefore.length() - 1;
        while (j >= 0 && (textBefore.at(j).isLetterOrNumber()
                          || textBefore.at(j) == '_' || textBefore.at(j) == '-'
                          || textBefore.at(j) == '@'))
            j--;
        prefix = textBefore.mid(j + 1);
        break;
    }
    case TkzCtxCoord: {
        int lp = textBefore.lastIndexOf('(');
        if (lp >= 0) prefix = textBefore.mid(lp + 1);
        break;
    }
    case TkzCtxUserCmd: {
        int lastSlash = textBefore.lastIndexOf('\\');
        if (lastSlash >= 0) prefix = textBefore.mid(lastSlash);
        break;
    }
    default:
        break;
    }

    if (ctx == TkzCtxWord) {
        int sl = textBefore.lastIndexOf(QLatin1Char('\\'));
        if (sl >= 0 && sl < static_cast<int>(textBefore.length()) - 1) {
            QString afterS = textBefore.mid(sl);
            // Only reroute to user-command completion when the backslash
            // directly introduces the token being typed (e.g. "\foo"). Without
            // this, an earlier command on the retained context (e.g.
            // "\draw ... grid") would hijack a plain path word like "grid" with
            // a bogus prefix and suppress completion entirely.
            static const QRegularExpression cmdTokenRe(
                QStringLiteral("^\\\\[A-Za-z@]+$"));
            if (cmdTokenRe.match(afterS).hasMatch()) {
                ctx = TkzCtxUserCmd;
                prefix = afterS;
            }
        }
    }

    if (!m_completers.contains(ctx)) return;
    QCompleter *comp = m_completers[ctx];

    if (ctx != m_activeContext && m_activeContext != TkzCtxNone) {
        if (m_completers.contains(m_activeContext))
            m_completers[m_activeContext]->popup()->hide();
    }
    m_activeContext = ctx;

    comp->setCompletionPrefix(prefix);

    if (prefix.isEmpty() && ctx != TkzCtxCmd && ctx != TkzCtxBeg
        && ctx != TkzCtxEnd && ctx != TkzCtxLib && ctx != TkzCtxBrk
        && ctx != TkzCtxDot && ctx != TkzCtxCoord) {
        comp->popup()->hide();
        return;
    }

    if (comp->completionCount() == 0) {
        comp->popup()->hide();
        return;
    }

    QRect cr = m_editor->cursorRect();
    cr.setWidth(comp->popup()->sizeHintForColumn(0)
                + comp->popup()->verticalScrollBar()->sizeHint().width() + 30);
    comp->complete(cr);

    if (comp->completionCount() > 0 && comp->popup())
        comp->popup()->setCurrentIndex(comp->completionModel()->index(0, 0));
}

bool TikzCompleter::handleCompletionKey(QKeyEvent *event)
{
    if (!isPopupVisible()) return false;
    QCompleter *comp = m_completers[m_activeContext];
    if (!comp) return false;

    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Tab: {
        QModelIndex idx = comp->popup()->currentIndex();
        if (!idx.isValid())
            idx = comp->completionModel()->index(0, 0);
        if (idx.isValid()) {
            QString completion = comp->completionModel()->data(idx).toString();
            QString prefix = comp->completionPrefix();
            int prefixLen = prefix.length();
            QTextCursor cursor = m_editor->textCursor();
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, prefixLen);
            cursor.insertText(completion);
            comp->popup()->hide();
            Context completedCtx = m_activeContext;
            m_activeContext = TkzCtxNone;

            // ── After completing \begin{env}: auto-insert \end{env} ──
            if (completedCtx == TkzCtxBeg && !completion.isEmpty()) {
                cursor = m_editor->textCursor();
                QTextDocument *doc = m_editor->document();
                int pos = cursor.position();
                bool hasAutoPair = (pos < doc->characterCount() - 1
                    && doc->characterAt(pos) == QLatin1Char('}'));

                QString lineIndent;
                {
                    QTextCursor lineCursor = cursor;
                    lineCursor.movePosition(QTextCursor::StartOfBlock,
                                            QTextCursor::KeepAnchor);
                    QString line = lineCursor.selectedText();
                    int indentEnd = 0;
                    while (indentEnd < line.length()
                           && (line.at(indentEnd) == ' '
                               || line.at(indentEnd) == '\t'))
                        indentEnd++;
                    lineIndent = line.left(indentEnd);
                }

                if (!hasAutoPair)
                    cursor.insertText(QLatin1String("}"));
                else
                    cursor.movePosition(QTextCursor::Right,
                                        QTextCursor::MoveAnchor, 1);
                int afterBeginPos = cursor.position();
                cursor.insertText(QLatin1String("\n") + lineIndent
                                  + QLatin1String("\\end{") + completion
                                  + QLatin1String("}"));
                // Position cursor after \begin{scope} on the begin line
                cursor.setPosition(afterBeginPos);
                m_editor->setTextCursor(cursor);
                if (m_docState)
                    m_docState->reparse(m_editor->document());
            } else if (completedCtx == TkzCtxEnd && !completion.isEmpty()) {
                cursor = m_editor->textCursor();
                QTextDocument *doc = m_editor->document();
                int pos = cursor.position();
                bool hasAutoPair = (pos < doc->characterCount() - 1
                    && doc->characterAt(pos) == QLatin1Char('}'));
                if (!hasAutoPair)
                    cursor.insertText(QLatin1String("}"));
                m_editor->setTextCursor(cursor);
            } else {
                m_editor->setTextCursor(cursor);
            }

            return true;
        }
        break;
    }
    case Qt::Key_Escape:
        comp->popup()->hide();
        m_activeContext = TkzCtxNone;
        return true;
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown: {
        QKeyEvent fakeEvent(QEvent::KeyPress, event->key(), event->modifiers());
        QCoreApplication::sendEvent(comp->popup(), &fakeEvent);
        return true;
    }
    default:
        break;
    }
    return false;
}
