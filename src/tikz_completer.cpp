#include "tikz_completer.h"
#include "tikz_words.h"
#include <QAbstractItemView>
#include <QScrollBar>
#include <QKeyEvent>
#include <QTextCursor>
#include <QRegularExpression>

TikzCompleter::TikzCompleter(QPlainTextEdit *editor, QObject *parent)
    : QObject(parent), m_editor(editor)
{
    initCompleters();
}

void TikzCompleter::initCompleters()
{
    auto makeCompleter = [this](Context ctx, const QStringList &words) {
        auto *model = new QStringListModel(words, this);
        m_models[ctx] = model;

        auto *comp = new QCompleter(this);
        comp->setModel(model);
        comp->setWidget(m_editor);
        comp->setCompletionMode(QCompleter::PopupCompletion);
        comp->setCaseSensitivity(Qt::CaseInsensitive);
        comp->setMaxVisibleItems(12);
        comp->popup()->setMinimumWidth(200);

        m_completers[ctx] = comp;
    };

    makeCompleter(TkzCtxCmd,     TikzWords::tikzCommands());
    makeCompleter(TkzCtxBeg, TikzWords::tikzEnvironments());
    makeCompleter(TkzCtxBrk,      TikzWords::tikzOptions());
    makeCompleter(TkzCtxDot,      TikzWords::tikzAnchors());
    makeCompleter(TkzCtxLib,     {"calc","arrows","shapes","positioning","patterns",
                                "decorations","intersections","through","angles",
                                "quotes","math","spy","shadows","fadings","fit",
                                "backgrounds","scopes","petri","er","automata",
                                "graphs","graphdrawing","lindenmayersystems",
                                "matrix","mindmap","folding","calendar",
                                "turtle","datavisualization","external","rdf",
                                "shapes.geometric","shapes.misc","shapes.arrows",
                                "shapes.symbols","shapes.multipart","shapes.callouts",
                                "decorations.pathmorphing","decorations.pathreplacing",
                                "decorations.markings","decorations.fractals",
                                "decorations.text","decorations.footprints",
                                "plotmarks","chains","circuits","circuits.logic",
                                "circuits.logic.IEC","circuits.logic.US",
                                "circuits.ee","circuits.ee.IEC","circuits.pid",
                                "circuits.pid.IEC","pgfplots.units",
                                "pgfplots.colorbrewer","3d","perspective",
                                "bending","svg.path","tikzmark","calligraphy",
                                "animations","fixedpointarithmetic","fpu",
                                "nonlineartransformations","optics","patterns.meta",
                                "pgfplots.groupplots","pgfplots.dateplot",
                                "pgfplots.polar","pgfplots.smithchart",
                                "pgfplots.statistics","pgfplots.ternary",
                                "pgfplots.units","profiler","shadings",
                                "svg.path","through","tikzmark","transparency"});

    QStringList valueWords;
    valueWords << TikzWords::tikzColors()
               << TikzWords::tikzLineWidths()
               << TikzWords::tikzArrows()
               << TikzWords::tikzLineTypes()
               << TikzWords::tikzLineWidthValues();
    makeCompleter(TkzCtxEq, valueWords);

    QStringList allWords = TikzWords::allCompletableWords();
    makeCompleter(TkzCtxWord, allWords);
}

void TikzCompleter::setModelForContext(Context ctx, const QStringList &words)
{
    if (m_models.contains(ctx)) {
        m_models[ctx]->setStringList(words);
    }
}

void TikzCompleter::refreshParamWords(const QStringList &params)
{
    if (params.isEmpty()) return;
    QStringList words;
    for (const QString &p : params)
        words << ("@@" + p + "@@");
    setModelForContext(TkzCtxAt, words);
}

bool TikzCompleter::isPopupVisible() const
{
    if (m_activeContext == TkzCtxNone) return false;
    if (!m_completers.contains(m_activeContext)) return false;
    return m_completers[m_activeContext]->popup()->isVisible();
}

TikzCompleter::Context TikzCompleter::detectContext(const QString &textBefore) const
{
    if (textBefore.isEmpty()) return TkzCtxNone;

    QString text = textBefore;
    int len = text.length();
    if (len == 0) return TkzCtxNone;

    QChar lastChar = text.at(len - 1);

    if (lastChar == '\\' && (len == 1 || text.at(len - 2).isSpace()))
        return TkzCtxCmd;

    static const QRegularExpression beginRe(QStringLiteral("\\\\begin\\s*\\{"));
    int lastBeginIdx = text.lastIndexOf(beginRe);
    if (lastBeginIdx >= 0) {
        QString afterBegin = text.mid(lastBeginIdx);
        int openBraces = afterBegin.count('{');
        int closeBraces = afterBegin.count('}');
        if (openBraces > closeBraces)
            return TkzCtxBeg;
    }

    if (text.contains(QRegularExpression(QStringLiteral("\\\\usetikzlibrary\\s*\\{")))) {
        int idx = text.lastIndexOf(QRegularExpression(QStringLiteral("\\\\usetikzlibrary")));
        QString after = text.mid(idx);
        int openBraces = after.count('{');
        int closeBraces = after.count('}');
        if (openBraces > closeBraces)
            return TkzCtxLib;
    }

    if (lastChar == '=') {
        int eqIdx = text.lastIndexOf('=');
        if (eqIdx > 0) {
            QString beforeKey = text.left(eqIdx);
            if (beforeKey.contains(QRegularExpression(QStringLiteral("[\\w\\s]"))))
                return TkzCtxEq;
        }
    }

    if (lastChar == '.') {
        if (len >= 2 && text.at(len - 2).isLetterOrNumber())
            return TkzCtxDot;
    }

    {
        int atIdx = text.lastIndexOf("@@");
        if (atIdx >= 0) {
            QString afterAt = text.mid(atIdx + 2);
            if (!afterAt.contains("@@"))
                return TkzCtxAt;
        }
    }

    int bracketDepth = 0;
    int braceDepth = 0;
    for (int i = len - 1; i >= 0; --i) {
        QChar ch = text.at(i);
        if (ch == '}') braceDepth++;
        else if (ch == '{') { if (braceDepth > 0) braceDepth--; }
        else if (ch == ']' && braceDepth == 0) bracketDepth++;
        else if (ch == '[' && braceDepth == 0) {
            if (bracketDepth > 0) bracketDepth--;
            else {
                // Check if this bracket belongs to a command
                static const QRegularExpression cmdRe(QStringLiteral("\\\\([a-zA-Z@]+)"));
                QString beforeBracket = text.left(i);
                QRegularExpressionMatch m = cmdRe.match(beforeBracket);
                if (m.hasMatch() || beforeBracket.contains('['))
                    return TkzCtxBrk;
                else
                    return TkzCtxBrk;
            }
        }
    }

    if (lastChar.isLetterOrNumber() || lastChar == '_') {
        static const QRegularExpression wordBoundary(QStringLiteral("[\\s\\\\\\[\\{,]"));
        int wordStart = text.lastIndexOf(wordBoundary);
        if (wordStart >= 0) {
            QChar beforeChar = text.at(wordStart);
            if (beforeChar == '\\') return TkzCtxCmd;
        }
        QString word;
        int j = len - 1;
        while (j >= 0 && (text.at(j).isLetterOrNumber() || text.at(j) == '_'
                          || text.at(j) == '-' || text.at(j) == '@'))
            j--;
        word = text.mid(j + 1);
        if (word.length() >= 2)
            return TkzCtxWord;
    }

    return TkzCtxNone;
}

void TikzCompleter::tryComplete()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    QString lineBefore = cursor.selectedText();

    QString textBefore;
    if (!lineBefore.isEmpty()) {
        int cursorPos = m_editor->textCursor().positionInBlock();
        if (cursorPos < lineBefore.length())
            textBefore = lineBefore.left(cursorPos);
        else
            textBefore = lineBefore;
    }

    Context ctx = detectContext(textBefore);
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
    case TkzCtxLib: {
        int lastBrace = textBefore.lastIndexOf('{');
        if (lastBrace >= 0) {
            int closePos = textBefore.indexOf('}', lastBrace);
            if (closePos < 0)
                prefix = textBefore.mid(lastBrace + 1);
            else
                prefix = textBefore.mid(lastBrace + 1, closePos - lastBrace - 1);
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
            QString afterEq = textBefore.mid(eqIdx + 1).trimmed();
            if (!afterEq.isEmpty()) {
                // Check for key hint
                QString beforeKey = textBefore.left(eqIdx).trimmed();
                int keyStart = beforeKey.lastIndexOf(',');
                if (keyStart < 0) keyStart = beforeKey.lastIndexOf('[');
                if (keyStart < 0) keyStart = -1;
                QString keyName = beforeKey.mid(keyStart + 1).trimmed();
                const auto &hints = TikzWords::tikzValueHints();
                for (const auto &pair : hints) {
                    if (keyName.compare(pair.first, Qt::CaseInsensitive) == 0
                        || keyName.endsWith(pair.first, Qt::CaseInsensitive)) {
                        if (!pair.second.isEmpty()) {
                            setModelForContext(TkzCtxEq, pair.second);
                        }
                        break;
                    }
                }
            }
            prefix = afterEq;
        }
        break;
    }
    case TkzCtxAt: {
        int atIdx = textBefore.lastIndexOf("@@");
        if (atIdx >= 0)
            prefix = textBefore.mid(atIdx + 2);
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
    default:
        break;
    }

    if (!m_completers.contains(ctx)) return;

    QCompleter *comp = m_completers[ctx];
    m_activeContext = ctx;

    QString oldPrefix = comp->completionPrefix();
    comp->setCompletionPrefix(prefix);

    if (prefix.isEmpty() && ctx != TkzCtxCmd && ctx != TkzCtxBeg && ctx != TkzCtxLib) {
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

    if (comp->completionCount() > 0 && comp->popup()) {
        comp->popup()->setCurrentIndex(comp->completionModel()->index(0, 0));
    }
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
