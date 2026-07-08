#include "tikz_completer.h"
#include "tikz_words.h"
#include "tikz_keywords.h"
#include "tikz_document_state.h"
#include <QAbstractItemView>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTextCursor>
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

void TikzCompleter::tryComplete()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    QString lineBefore = cursor.selectedText();

    QString textBefore;
    int cursorPos = m_editor->textCursor().positionInBlock();
    if (cursorPos < lineBefore.length())
        textBefore = lineBefore.left(cursorPos);
    else
        textBefore = lineBefore;

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
    case TkzCtxBeg: {
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
        int eqIdx = textBefore.lastIndexOf('=');
        if (eqIdx >= 0) {
            QString beforeKey = textBefore.left(eqIdx).trimmed();
            int keyStart = beforeKey.lastIndexOf(',');
            if (keyStart < 0) keyStart = beforeKey.lastIndexOf('[');
            QString keyName = beforeKey.mid(keyStart + 1).trimmed();
            updateEqModel(keyName);
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
    case TkzCtxWord: {
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
            if (!afterS.isEmpty() && afterS.at(1).isLetter()) {
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
        && ctx != TkzCtxLib && ctx != TkzCtxBrk && ctx != TkzCtxDot
        && ctx != TkzCtxCoord) {
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
            m_activeContext = TkzCtxNone;
            m_editor->setTextCursor(cursor);
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
