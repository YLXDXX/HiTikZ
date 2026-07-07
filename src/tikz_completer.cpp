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
        QAbstractItemView *popup = comp->popup();
        popup->setMinimumWidth(200);
        popup->winId();
        if (auto *ph = popup->windowHandle())
            if (auto *eh = m_editor->window()->windowHandle())
                ph->setTransientParent(eh);
        m_completers[ctx] = comp;
    };

    QStringList cmdWords;
    for (const QString &c : TikzWords::tikzCommands())
        cmdWords << ("\\" + c);
    makeCompleter(TkzCtxCmd, cmdWords);
    makeCompleter(TkzCtxBeg, TikzWords::tikzEnvironments());
    makeCompleter(TkzCtxBrk, TikzWords::tikzOptions());

    {
        QStringList dotWords = TikzWords::tikzAnchors();
        dotWords << TikzWords::tikzKeyHandlers();
        dotWords.removeDuplicates();
        makeCompleter(TkzCtxDot, dotWords);
    }

    makeCompleter(TkzCtxLib, {
        "calc","arrows","shapes","positioning","patterns",
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
        "profiler","shadings","transparency",
        "arrows.meta","trees","topaths","graphs.standard",
        "babel","cd","circuitikz","shapes.gates.logic",
        "shapes.gates.logic.IEC","shapes.gates.logic.US"
    });

    QStringList arrowVals = TikzWords::tikzArrows();
    arrowVals.erase(std::remove_if(arrowVals.begin(), arrowVals.end(),
        [](const QString &s) { return s.contains(' '); }), arrowVals.end());
    QStringList valueWords;
    valueWords << TikzWords::tikzColors()
               << TikzWords::tikzLineWidths()
               << arrowVals
               << TikzWords::tikzLineTypes()
               << TikzWords::tikzLineWidthValues();
    makeCompleter(TkzCtxEq, valueWords);

    makeCompleter(TkzCtxWord, TikzWords::allCompletableWords());
    makeCompleter(TkzCtxAt, {});
    makeCompleter(TkzCtxCoord, {});
    makeCompleter(TkzCtxUserCmd, {});
}

void TikzCompleter::setDocumentState(TikzDocumentState *state)
{
    m_docState = state;
}

void TikzCompleter::setModelForContext(Context ctx, const QStringList &words)
{
    if (m_models.contains(ctx))
        m_models[ctx]->setStringList(words);
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
    int len = textBefore.length();
    if (len == 0) return TkzCtxNone;

    QChar lastChar = textBefore.at(len - 1);

    if (lastChar == '\\' && (len == 1 || textBefore.at(len - 2).isSpace()))
        return TkzCtxCmd;

    static const QRegularExpression beginRe(QStringLiteral("\\\\begin\\s*\\{"));
    int lastBeginIdx = textBefore.lastIndexOf(beginRe);
    if (lastBeginIdx >= 0) {
        QString afterBegin = textBefore.mid(lastBeginIdx);
        if (afterBegin.count('{') > afterBegin.count('}'))
            return TkzCtxBeg;
    }

    if (textBefore.contains(QRegularExpression(QStringLiteral("\\\\usetikzlibrary\\s*\\{")))) {
        int idx = textBefore.lastIndexOf(QRegularExpression(QStringLiteral("\\\\usetikzlibrary")));
        QString after = textBefore.mid(idx);
        if (after.count('{') > after.count('}'))
            return TkzCtxLib;
    }

    if (lastChar == '=') {
        int eqIdx = textBefore.lastIndexOf('=');
        if (eqIdx > 0 && textBefore.left(eqIdx).contains(QRegularExpression(QStringLiteral("[\\w\\s]"))))
            return TkzCtxEq;
    }

    if (lastChar == '.') {
        if (len >= 2 && textBefore.at(len - 2).isLetterOrNumber())
            return TkzCtxDot;
    }

    {
        int dotIdx = textBefore.lastIndexOf('.');
        if (dotIdx >= 0 && dotIdx < len - 1) {
            QChar beforeDot = textBefore.at(dotIdx - 1);
            if (beforeDot.isLetterOrNumber() || beforeDot == '/') {
                QString afterDot = textBefore.mid(dotIdx + 1);
                bool allWordChars = true;
                for (const QChar &c : afterDot) {
                    if (c.isSpace()) break;
                    if (!c.isLetterOrNumber() && c != '_' && c != '-')
                    { allWordChars = false; break; }
                }
                if (allWordChars && afterDot.length() >= 1)
                    return TkzCtxDot;
            }
        }
    }

    {
        int atCount = textBefore.count("@@");
        if (atCount > 0 && atCount % 2 == 1)
            return TkzCtxAt;
        if (lastChar == '@' && !textBefore.endsWith("@@"))
            return TkzCtxAt;
    }

    // Check for coordinate context: (name after an opening (
    {
        int lastOpenParen = textBefore.lastIndexOf('(');
        int lastCloseParen = textBefore.lastIndexOf(')');
        if (lastOpenParen > lastCloseParen && lastOpenParen >= 0) {
            QString afterParen = textBefore.mid(lastOpenParen + 1);
            if (!afterParen.contains(',') && !afterParen.contains(' ') &&
                afterParen.length() >= 0 && afterParen.length() < 30)
                return TkzCtxCoord;
        }
    }

    int bracketDepth = 0;
    int braceDepth = 0;
    for (int i = len - 1; i >= 0; --i) {
        QChar ch = textBefore.at(i);
        if (ch == '}') braceDepth++;
        else if (ch == '{') { if (braceDepth > 0) braceDepth--; }
        else if (ch == ']' && braceDepth == 0) bracketDepth++;
        else if (ch == '[' && braceDepth == 0) {
            if (bracketDepth > 0) bracketDepth--;
            else {
                QString afterBracket = textBefore.mid(i + 1);
                if (afterBracket.contains(']'))
                    continue;
                int eqIdx = afterBracket.lastIndexOf('=');
                int commaIdx = afterBracket.lastIndexOf(',');
                if (eqIdx >= 0 && (commaIdx < 0 || commaIdx <= eqIdx))
                    return TkzCtxEq;
                return TkzCtxBrk;
            }
        }
    }

    if (lastChar.isLetterOrNumber() || lastChar == '_') {
        static const QRegularExpression wordBoundary(QStringLiteral("[\\s\\\\\\[\\{,(\"]"));
        int wordStart = textBefore.lastIndexOf(wordBoundary);
        if (wordStart >= 0) {
            QChar bc = textBefore.at(wordStart);
            if (bc == '\\') return TkzCtxCmd;
            if (bc == '(') return TkzCtxCoord;
        }
        QString word;
        int j = len - 1;
        while (j >= 0 && (textBefore.at(j).isLetterOrNumber()
                          || textBefore.at(j) == '_' || textBefore.at(j) == '-'
                          || textBefore.at(j) == '@'))
            j--;
        word = textBefore.mid(j + 1);
        if (word.length() >= 2)
            return TkzCtxWord;
    }

    return TkzCtxNone;
}

QStringList TikzCompleter::buildBrkCandidates(Context /*ctx*/)
{
    // Build option candidates filtered by current env/cmd/libs
    QStringList candidates;
    if (!m_docState) {
        candidates = TikzWords::tikzOptions();
    } else {
        QTextCursor cursor = m_editor->textCursor();
        int pos = cursor.position();
        QString env = m_docState->currentEnvName(pos);
        // Use current text context to determine command
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        QString lineBefore = cursor.selectedText();
        int cp = m_editor->textCursor().positionInBlock();
        QString cmdName;
        static const QRegularExpression cmdLineRe(
            QStringLiteral("\\\\(draw|path|fill|filldraw|shade|shadedraw|node|pic|edge)\\b"));
        QRegularExpressionMatch cm = cmdLineRe.match(lineBefore);
        if (cm.hasMatch() && cm.capturedStart() < cp)
            cmdName = cm.captured(1);

        auto optionKws = TikzKeywords::TikzKeywordDB::instance().filter(
            env, cmdName, m_docState->activeLibs(),
            TikzKeywords::Category::Option);
        for (auto *kw : optionKws)
            candidates << kw->name;

        auto colorKws = TikzKeywords::TikzKeywordDB::instance().filter(
            env, cmdName, m_docState->activeLibs(),
            TikzKeywords::Category::Color);
        for (auto *kw : colorKws)
            candidates << kw->name;

        auto shapeKws = TikzKeywords::TikzKeywordDB::instance().filter(
            env, cmdName, m_docState->activeLibs(),
            TikzKeywords::Category::Shape);
        for (auto *kw : shapeKws)
            candidates << kw->name;

        auto lwKws = TikzKeywords::TikzKeywordDB::instance().filter(
            env, cmdName, m_docState->activeLibs(),
            TikzKeywords::Category::LineWidth);
        for (auto *kw : lwKws)
            candidates << kw->name;

        auto ltKws = TikzKeywords::TikzKeywordDB::instance().filter(
            env, cmdName, m_docState->activeLibs(),
            TikzKeywords::Category::LineType);
        for (auto *kw : ltKws)
            candidates << kw->name;

        auto arrKws = TikzKeywords::TikzKeywordDB::instance().filter(
            env, cmdName, m_docState->activeLibs(),
            TikzKeywords::Category::Arrow);
        for (auto *kw : arrKws) {
            if (!kw->name.contains(' '))
                candidates << kw->name;
        }

        auto decKws = TikzKeywords::TikzKeywordDB::instance().filter(
            env, cmdName, m_docState->activeLibs(),
            TikzKeywords::Category::Decoration);
        for (auto *kw : decKws)
            candidates << kw->name;

        auto patKws = TikzKeywords::TikzKeywordDB::instance().filter(
            env, cmdName, m_docState->activeLibs(),
            TikzKeywords::Category::Pattern);
        for (auto *kw : patKws)
            candidates << kw->name;
    }

    // Add user styles
    if (m_docState) {
        for (const auto &s : m_docState->userStyles().keys())
            candidates << s;
        for (const auto &c : m_docState->definedColors().keys())
            candidates << c;
    }
    candidates.removeDuplicates();
    candidates.sort(Qt::CaseInsensitive);
    return candidates;
}

void TikzCompleter::updateBrkModel()
{
    setModelForContext(TkzCtxBrk, buildBrkCandidates(TkzCtxBrk));
}

void TikzCompleter::updateEqModel(const QString &keyName)
{
    QStringList vals;
    const auto *kw = TikzKeywords::TikzKeywordDB::instance().find(
        keyName, TikzKeywords::Category::Option);
    if (kw && !kw->valueHints.isEmpty())
        vals = kw->valueHints;

    QStringList extraHints = TikzKeywords::TikzKeywordDB::instance().valueHintsFor(keyName);
    vals << extraHints;

    vals << TikzWords::tikzColors();
    vals << TikzWords::tikzLineWidths();
    {
        auto arrVals = TikzWords::tikzArrows();
        arrVals.erase(std::remove_if(arrVals.begin(), arrVals.end(),
            [](const QString &s) { return s.contains(' '); }), arrVals.end());
        vals << arrVals;
    }
    vals << TikzWords::tikzLineTypes();
    vals << TikzWords::tikzLineWidthValues();
    vals.removeDuplicates();
    setModelForContext(TkzCtxEq, vals);
}

void TikzCompleter::updateUserModels()
{
    if (!m_docState) return;

    // Update TkzCtxCoord model with user coords and nodes
    QStringList coords;
    for (const auto &c : m_docState->userCoordinates())
        coords << c;
    for (const auto &n : m_docState->userNodes())
        coords << n;
    coords.removeDuplicates();
    if (!coords.isEmpty())
        setModelForContext(TkzCtxCoord, coords);

    // Update TkzCtxUserCmd with user commands
    QStringList ucmds;
    for (const auto &c : m_docState->userCommands())
        ucmds << c;
    if (!ucmds.isEmpty())
        setModelForContext(TkzCtxUserCmd, ucmds);

    // Merge user styles into TkzCtxBrk
    updateBrkModel();
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
