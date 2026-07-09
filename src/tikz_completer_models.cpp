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

    // Commands are dynamically populated in updateUserModels
    makeCompleter(TkzCtxCmd, {});
    makeCompleter(TkzCtxBeg, TikzWords::tikzEnvironments());
    makeCompleter(TkzCtxBrk, TikzWords::tikzOptions());

    {
        QStringList dotWords = TikzWords::tikzAnchors();
        dotWords << TikzWords::tikzKeyHandlers();
        dotWords.removeDuplicates();
        makeCompleter(TkzCtxDot, dotWords);
    }

    makeCompleter(TkzCtxLib, TikzWords::tikzLibraries());

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
    makeCompleter(TkzCtxPathWord, TikzWords::tikzPathOperations());
    makeCompleter(TkzCtxAt, {});
    makeCompleter(TkzCtxCoord, {});
    makeCompleter(TkzCtxUserCmd, {});
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

QString TikzCompleter::commandForOptionContext(const QString &lineBefore, int cursorCol)
{
    static const QRegularExpression cmdLineRe(
        QStringLiteral("\\\\(draw|path|fill|filldraw|shade|shadedraw|node|pic|edge)\\b"));
    // Use the LAST matching command before the cursor, so on lines with several
    // commands (e.g. "\draw (0,0) -- \node[..." ) the option context reflects
    // the command the cursor is actually inside.
    QRegularExpressionMatchIterator it = cmdLineRe.globalMatch(lineBefore);
    QString cmdName;
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        if (m.capturedStart() < cursorCol)
            cmdName = m.captured(1);
        else
            break;
    }
    return cmdName;
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
        QString cmdName = commandForOptionContext(lineBefore, cp);

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

QStringList TikzCompleter::eqCandidatesForKey(const QString &keyName) const
{
    QStringList vals;
    const QString lower = keyName.toLower();

    // Color-valued keys must offer the full palette (built-in colors +
    // user \definecolor/\colorlet), never a hardcoded subset. Keys such as
    // "color", "draw", "fill", "left color", "ball color", ... all match.
    const bool isColorKey = lower.contains("color")
                            || lower == "draw" || lower == "fill";

    if (isColorKey) {
        vals << TikzWords::tikzColors();
        if (m_docState) {
            const auto colorKeys = m_docState->definedColors().keys();
            for (const auto &c : colorKeys)
                vals << c;
        }
    } else {
        const auto *kw = TikzKeywords::TikzKeywordDB::instance().find(
            keyName, TikzKeywords::Category::Option);
        if (kw && !kw->valueHints.isEmpty()) {
            vals = kw->valueHints;
        } else if (lower.contains("width") || lower.contains("sep")
                   || lower.contains("distance") || lower.contains("size")
                   || lower.contains("radius")) {
            vals << TikzWords::tikzLineWidthValues();
        } else if (lower.contains("arrow") || lower == ">" || lower == ">=") {
            vals << TikzWords::tikzArrows();
        } else if (lower == "pattern") {
            vals << TikzWords::tikzPatternNames();
        } else if (lower == "decoration") {
            vals << TikzWords::tikzDecorationNames();
        }
    }

    // Positioning keys (above/below/left/right and their =of variants): offer
    // coordinate/node names as completions, both bare and "of "-prefixed, so
    // \node[below=of wai...] completes to "of waiting 1".
    {
        static const QSet<QString> posKeys = {
            "above", "above left", "above right",
            "below", "below left", "below right",
            "left", "right",
            "above=of", "below=of", "left=of", "right=of"
        };
        if (posKeys.contains(lower) && m_docState) {
            for (const auto &c : m_docState->userCoordinates())
                vals << (QLatin1String("of ") + c);
            for (const auto &n : m_docState->userNodes())
                vals << (QLatin1String("of ") + n);
            for (const auto &c : m_docState->userCoordinates())
                vals << c;
            for (const auto &n : m_docState->userNodes())
                vals << n;
        }
    }

    vals.removeDuplicates();
    vals.sort(Qt::CaseInsensitive);
    return vals;
}

void TikzCompleter::updateEqModel(const QString &keyName)
{
    setModelForContext(TkzCtxEq, eqCandidatesForKey(keyName));
}

void TikzCompleter::updateUserModels()
{
    if (!m_docState) return;

    QTextCursor cursor = m_editor->textCursor();
    int pos = cursor.position();
    QString env = m_docState->currentEnvName(pos);

    // Update TkzCtxCoord model with user coords and nodes
    QStringList coords;
    for (const auto &c : m_docState->userCoordinates())
        coords << c;
    for (const auto &n : m_docState->userNodes())
        coords << n;
    coords.removeDuplicates();
    setModelForContext(TkzCtxCoord, coords);

    // Update TkzCtxUserCmd with user commands and foreach variables
    QStringList ucmds;
    for (const auto &c : m_docState->userCommands())
        ucmds << c;
    for (const auto &v : m_docState->foreachVars())
        ucmds << (QLatin1String("\\") + v);
    ucmds.removeDuplicates();
    setModelForContext(TkzCtxUserCmd, ucmds);

    // Filter commands by environment and active libs
    QStringList cmds;
    auto cmdKws = TikzKeywords::TikzKeywordDB::instance().filter(
        env, QString(), m_docState->activeLibs(), TikzKeywords::Category::Command);
    for (auto *kw : cmdKws)
        cmds << (QLatin1String("\\") + kw->name);
    cmds << ucmds;
    cmds.removeDuplicates();
    cmds.sort(Qt::CaseInsensitive);
    setModelForContext(TkzCtxCmd, cmds);
    // Keep TkzCtxUserCmd in sync so it is usable when detectContext ever
    // chooses to route backslash+letter to TkzCtxUserCmd or when that context
    // is activated programmatically.
    setModelForContext(TkzCtxUserCmd, cmds);

    // Also update TkzCtxWord so typed words appear in completion
    QStringList wordModel;
    for (auto *kw : cmdKws) wordModel << kw->name;
    wordModel << TikzWords::allCompletableWords();
    wordModel << ucmds;
    wordModel.removeDuplicates();
    wordModel.sort(Qt::CaseInsensitive);
    setModelForContext(TkzCtxWord, wordModel);

    // Merge user styles into TkzCtxBrk
    updateBrkModel();
}
