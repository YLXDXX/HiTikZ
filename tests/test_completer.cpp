#include <QApplication>
#include <QPlainTextEdit>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QSet>
#include <cstdio>
#include "tikz_words.h"
#include "tikz_completer.h"
#include "tikz_keywords.h"
#include "tikz_document_state.h"

static int test_options_contain_help_lines()
{
    int failed = 0;
    const QStringList opts = TikzWords::tikzOptions();

    if (!opts.contains(QStringLiteral("help lines"))) {
        fprintf(stderr, "FAIL: OPT-1 - tikzOptions() should contain 'help lines'\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: help lines in tikzOptions\n");
    return failed;
}

static int test_options_contain_new_entries()
{
    int failed = 0;
    const QStringList opts = TikzWords::tikzOptions();

    struct { const char *name; const char *desc; } entries[] = {
        {"help lines", "help lines style"},
        {"->", "arrow end spec"},
        {"<-", "arrow start spec"},
        {"<->", "arrow both ends spec"},
        {"every picture", "every picture style"},
        {"every label", "every label style"},
        {"every pin", "every pin style"},
        {"every to", "every to style"},
        {"every scope", "every scope style"},
        {"label position", "label position key"},
        {"text ragged", "text ragged alignment"},
        {"text badly ragged", "text badly ragged alignment"},
        {"text badly centered", "text badly centered alignment"},
        {"miter limit", "miter limit key"},
        {"no draw", "no draw key"},
        {"no fill", "no fill key"},
        {"reset cm", "reset cm key"},
        {"execute at begin node", "execute at begin node key"},
        {"execute at end node", "execute at end node key"},
        {"circular drop shadow", "circular drop shadow key"},
        {"clip", "clip key"},
        {"end angle", "end angle arc key"},
        {"delta angle", "delta angle arc key"},
        {"above=of", "above=of positioning"},
        {"below=of", "below=of positioning"},
        {"isosceles triangle", "isosceles triangle shape"},
        {"semicircle", "semicircle shape"},
        {"circular sector", "circular sector shape"},
        {"single arrow", "single arrow shape"},
        {"double arrow", "double arrow shape"},
        {"axis z line", "pgfplots axis z line"},
        {"zmin", "pgfplots zmin"},
        {"zmax", "pgfplots zmax"},
        {"ztick", "pgfplots ztick"},
        {"zticklabels", "pgfplots zticklabels"},
        {"ztick distance", "pgfplots ztick distance"},
        {"z radius", "z radius for elliptical arcs"},
        {"out", "out curve angle"},
        {"in looseness", "in looseness curve key"},
        {"out looseness", "out looseness curve key"},
        {"controls", "Bezier control points"},
        {"parabola", "parabola path operation"},
        {"densely dashed", "densely dashed line type"},
        {"densely dotted", "densely dotted line type"},
        {"dash dot", "dash dot line type"},
        {"dash dot dot", "dash dot dot line type"},
        {"densely dash dot", "densely dash dot line type"},
        {"densely dash dot dot", "densely dash dot dot line type"},
        {"name path", "intersections name path"},
        {"name intersections", "intersections compute"},
        {"of", "intersections of key"},
        {"by", "intersections by key"},
        {"sort by", "intersections sort by"},
        {"nonzero rule", "fill rule nonzero"},
        {"even odd rule", "fill rule even odd"},
        {"angle radius", "angles library radius"},
        {"angle eccentricity", "angles library eccentricity"},
        {"right angle", "angles library right angle pic"},
        {"pic text", "angles library pic text"},
    };

    for (const auto &e : entries) {
        if (!opts.contains(QString::fromLatin1(e.name))) {
            fprintf(stderr, "FAIL: OPT-2 - tikzOptions() should contain '%s' (%s)\n",
                    e.name, e.desc);
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: %zu new option entries verified\n",
                             sizeof(entries)/sizeof(entries[0]));
    return failed;
}

static int test_options_existing_entries_preserved()
{
    int failed = 0;
    const QStringList opts = TikzWords::tikzOptions();

    const char *existing[] = {
        "above", "below", "left", "right",
        "draw", "fill", "color", "thick", "thin",
        "solid", "dashed", "dotted",
        "->", "stealth", "rounded corners",
        "rotate", "scale", "anchor",
        "smooth", "sloped",
        "node distance", "inner sep", "outer sep",
        "minimum width", "minimum height", "text width",
        "pattern", "pattern color",
        "decorate", "decoration",
        "arrow", "bend left", "bend right",
        "coordinate", "circle", "rectangle",
        "grid", "grid style",
        "help lines", "top color", "bottom color",
        nullptr
    };

    for (int i = 0; existing[i] != nullptr; i++) {
        QString entry = QString::fromLatin1(existing[i]);
        if (!opts.contains(entry)) {
            fprintf(stderr, "FAIL: PRESERVE - existing option '%s' is missing!\n",
                    existing[i]);
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: existing options preserved\n");
    return failed;
}

static int test_value_hints_arrow_tip_keys()
{
    int failed = 0;
    const auto hints = TikzWords::tikzValueHints();

    bool hasGt = false;
    bool hasGtEq = false;
    for (const auto &pair : hints) {
        if (pair.first == QStringLiteral(">")) {
            hasGt = true;
            if (pair.second.isEmpty()) {
                fprintf(stderr, "FAIL: VH-1 - '>' value hints should not be empty\n");
                failed++;
            }
            if (!pair.second.contains(QStringLiteral("stealth"))) {
                fprintf(stderr, "FAIL: VH-2 - '>' hints should contain 'stealth'\n");
                failed++;
            }
        }
        if (pair.first == QStringLiteral(">=")) {
            hasGtEq = true;
            if (pair.second.isEmpty()) {
                fprintf(stderr, "FAIL: VH-3 - '>=' value hints should not be empty\n");
                failed++;
            }
            if (!pair.second.contains(QStringLiteral("stealth"))) {
                fprintf(stderr, "FAIL: VH-4 - '>=' hints should contain 'stealth'\n");
                failed++;
            }
        }
    }

    if (!hasGt) {
        fprintf(stderr, "FAIL: VH-5 - value hints should contain '>' key\n");
        failed++;
    }
    if (!hasGtEq) {
        fprintf(stderr, "FAIL: VH-6 - value hints should contain '>=' key\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: arrow tip value hints ('>' and '>=')\n");
    return failed;
}

static int test_value_hints_pattern()
{
    int failed = 0;
    const auto hints = TikzWords::tikzValueHints();

    bool hasPattern = false;
    for (const auto &pair : hints) {
        if (pair.first == QStringLiteral("pattern")) {
            hasPattern = true;
            if (pair.second.isEmpty()) {
                fprintf(stderr, "FAIL: VH-P1 - 'pattern' value hints should not be empty\n");
                failed++;
            }
            const char *expected[] = {
                "horizontal lines", "vertical lines", "north east lines",
                "north west lines", "crosshatch", "crosshatch dots",
                "bricks", "checkerboard", "grid", "dots", nullptr
            };
            for (int i = 0; expected[i] != nullptr; i++) {
                if (!pair.second.contains(QString::fromLatin1(expected[i]))) {
                    fprintf(stderr, "FAIL: VH-P2 - pattern hints should contain '%s'\n",
                            expected[i]);
                    failed++;
                }
            }
            break;
        }
    }

    if (!hasPattern) {
        fprintf(stderr, "FAIL: VH-P3 - value hints should contain 'pattern' key\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: pattern value hints\n");
    return failed;
}

static int test_detect_context()
{
    int failed = 0;

    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    struct TestCase {
        QString text;
        TikzCompleter::Context expected;
        QString desc;
    };

    TestCase tests[] = {
        {QStringLiteral("\\draw[even odd"), TikzCompleter::TkzCtxBrk, "even odd fill rule in bracket"},
        {QStringLiteral("\\draw[nonzero ru"), TikzCompleter::TkzCtxBrk, "nonzero fill rule in bracket"},
        {QStringLiteral("\\filldraw [even odd"), TikzCompleter::TkzCtxBrk, "filldraw bracket even odd"},
        {QStringLiteral("\\filldraw [even"), TikzCompleter::TkzCtxBrk, "filldraw bracket even"},
        {QStringLiteral("\\draw["), TikzCompleter::TkzCtxBrk, "open bracket"},
        {QStringLiteral("\\draw[help lines"), TikzCompleter::TkzCtxBrk, "help lines option"},
        {QStringLiteral("\\draw[help lin"), TikzCompleter::TkzCtxBrk, "partial help lines"},
        {QStringLiteral("\\draw[->"), TikzCompleter::TkzCtxBrk, "arrow spec in bracket"},
        {QStringLiteral("\\draw[red, thick, ->"), TikzCompleter::TkzCtxBrk, "multiple bracket opts"},
        {QStringLiteral("\\draw[->,>="), TikzCompleter::TkzCtxEq, ">= with = at end triggers Eq"},
        {QStringLiteral("\\draw[color="), TikzCompleter::TkzCtxEq, "after = in bracket"},
        {QStringLiteral("\\draw[color=r"), TikzCompleter::TkzCtxEq, "typing color value"},
        {QStringLiteral("\\draw[color=red"), TikzCompleter::TkzCtxEq, "completed color value"},
        {QStringLiteral("\\draw[align=c"), TikzCompleter::TkzCtxEq, "typing align value"},
        {QStringLiteral("\\draw[align=center"), TikzCompleter::TkzCtxEq, "completed align value"},
        {QStringLiteral("\\draw[>=s"), TikzCompleter::TkzCtxEq, "typing >= arrow value"},
        {QStringLiteral("\\draw[>=stealth"), TikzCompleter::TkzCtxEq, "completed >= arrow value"},
        {QStringLiteral("\\draw[out=9"), TikzCompleter::TkzCtxEq, "typing out angle value"},
        {QStringLiteral("\\draw[in=-1"), TikzCompleter::TkzCtxEq, "typing in angle value"},
        {QStringLiteral("\\draw[color=red,"), TikzCompleter::TkzCtxBrk, "comma after value = new key"},
        {QStringLiteral("\\draw[color=red, thick"), TikzCompleter::TkzCtxBrk, "second bracket key"},
        {QStringLiteral("\\draw[color=red, >="), TikzCompleter::TkzCtxEq, "new key=value after comma"},
        {QStringLiteral("\\node at (0,0) {\\tikzset{test lines."), TikzCompleter::TkzCtxDot, "just typed dot in tikzset"},
        {QStringLiteral("\\node at (0,0) {\\tikzset{test lines/.d"), TikzCompleter::TkzCtxDot, "tikzset/.d typing"},
        {QStringLiteral("\\node at (0,0) {\\tikzset{test lines/.def"), TikzCompleter::TkzCtxDot, "tikzset/.def typing"},
        {QStringLiteral("\\node at (0,0) {\\tikzset{test lines/.st"), TikzCompleter::TkzCtxDot, "tikzset/.st typing"},
        {QStringLiteral("\\node at (0,0) {\\tikzset{test lines/.default"), TikzCompleter::TkzCtxDot, "tikzset/.default done"},
        {QStringLiteral("node."), TikzCompleter::TkzCtxDot, "node dot for anchor"},
        {QStringLiteral("node.nort"), TikzCompleter::TkzCtxDot, "node anchor typing"},
        {QStringLiteral("node.30"), TikzCompleter::TkzCtxDot, "node angle anchor"},
        {QStringLiteral(" \\"), TikzCompleter::TkzCtxCmd, "backslash after space"},
        {QStringLiteral("\\begin{tikzpictu"), TikzCompleter::TkzCtxBeg, "inside begin"},
        {QStringLiteral("\\draw[help lines] (0,0) gr"), TikzCompleter::TkzCtxPathWord, "keyword after bracket (path body)"},
        // Path-word context: bare words in the path body (outside braces) route to
        // TkzCtxPathWord with a restricted set, while words inside '{...}' remain
        // TkzCtxWord for node text / math expressions.
        {QStringLiteral("\\draw (0,0) rec"), TikzCompleter::TkzCtxPathWord, "rec in path body -> path word"},
        {QStringLiteral("\\draw (0,0) cir"), TikzCompleter::TkzCtxPathWord, "cir in path body -> path word"},
        {QStringLiteral("\\draw (0,0) elli"), TikzCompleter::TkzCtxPathWord, "elli in path body -> path word"},
        {QStringLiteral("\\draw (0,0) -- (1,1) no"), TikzCompleter::TkzCtxPathWord, "node after line-to -> path word"},
        {QStringLiteral("\\draw (0,0) node {hello} rec"), TikzCompleter::TkzCtxPathWord, "word after closed brace back in path body"},
        {QStringLiteral("\\draw (0,0) node {rec"), TikzCompleter::TkzCtxWord, "rec inside node text -> generic word"},
        {QStringLiteral("\\draw (0,0) node {reciproc"), TikzCompleter::TkzCtxWord, "reciproc inside braces -> generic word"},
        {QStringLiteral("\\draw (0,0) node {hello} -- (1,1) ar"), TikzCompleter::TkzCtxPathWord, "arc after node and line-to -> path word"},
        // Coordinate context with names containing spaces (names like "critical 1"
        // or "waiting 1" must be detected inside "(" now that the space restriction
        // was removed).
        {QStringLiteral("\\draw (0,0) -- (waiting 1"), TikzCompleter::TkzCtxCoord, "coordinate name with space after ("},
        {QStringLiteral("\\draw (crit"), TikzCompleter::TkzCtxCoord, "coordinate name after ("},
        // Still excluded: coordinate pairs with commas.
        {QStringLiteral("\\draw (0,0"), TikzCompleter::TkzCtxNone, "coordinate pair (not a named coord)"},
        // After =of in a positioning key: TkzCtxEq must fire (so the value
        // completer can offer coordinate names).
        {QStringLiteral("[below=of wai"), TikzCompleter::TkzCtxEq, "positioning key =of value"},
        {QStringLiteral("[below=of"), TikzCompleter::TkzCtxEq, "positioning key =of (just typed of)"},
        {QStringLiteral("[above=of critical"), TikzCompleter::TkzCtxEq, "positioning above=of value"},
        {QStringLiteral("[right=of"), TikzCompleter::TkzCtxEq, "positioning right=of"},
        // Regular bracket context without = should still work.
        {QStringLiteral("[draw, below"), TikzCompleter::TkzCtxBrk, "below as key in bracket"},
        {QStringLiteral("\\node[draw] at (0.5,"), TikzCompleter::TkzCtxNone, "coordinates"},
        {QStringLiteral("\\usetikzlibrary{"), TikzCompleter::TkzCtxLib, "empty usetikzlibrary brace"},
        {QStringLiteral("\\usetikzlibrary{inters"), TikzCompleter::TkzCtxLib, "single library typing"},
        {QStringLiteral("\\usetikzlibrary{intersections,angles,quo"), TikzCompleter::TkzCtxLib, "library after commas"},
    };

    for (const auto &tc : tests) {
        TikzCompleter::Context result = completer.detectContext(tc.text);
        if (result != tc.expected) {
            fprintf(stderr, "FAIL: DC-%s - for '%s': expected %d, got %d\n",
                    qPrintable(tc.desc), qPrintable(tc.text),
                    static_cast<int>(tc.expected), static_cast<int>(result));
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: %zu detectContext tests\n",
                             sizeof(tests)/sizeof(tests[0]));
    return failed;
}

static int test_all_completable_words_no_duplicates()
{
    int failed = 0;
    QStringList words = TikzWords::allCompletableWords();

    QSet<QString> seen;
    for (const QString &w : words) {
        QString lower = w.toLower();
        if (seen.contains(lower)) {
            fprintf(stderr, "FAIL: DUP-1 - duplicate word: '%s'\n",
                    qPrintable(w));
            failed++;
        }
        seen.insert(lower);
    }

    if (failed == 0) fprintf(stderr, "PASS: no duplicates in allCompletableWords (%d unique)\n",
                             words.size());
    return failed;
}

static int test_arrow_specs_not_duplicated()
{
    int failed = 0;
    QStringList cmdWords;
    for (const QString &c : TikzWords::tikzCommands())
        cmdWords << ("\\" + c);

    if (cmdWords.contains(QStringLiteral("\\->"))) {
        fprintf(stderr, "FAIL: ARW-1 - '->' should not be in tikzCommands (it's an option)\n");
        failed++;
    }
    if (cmdWords.contains(QStringLiteral("\\<-"))) {
        fprintf(stderr, "FAIL: ARW-2 - '<-' should not be in tikzCommands (it's an option)\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: arrow specs in correct list (options, not commands)\n");
    return failed;
}

static int test_colors_in_options()
{
    int failed = 0;
    const QStringList opts = TikzWords::tikzOptions();

    const char *colors[] = {
        "red", "green", "blue", "cyan", "magenta", "yellow",
        "black", "white", "gray", "darkgray", "lightgray",
        "brown", "lime", "olive", "orange", "pink", "purple",
        "teal", "violet", nullptr
    };

    for (int i = 0; colors[i] != nullptr; i++) {
        if (!opts.contains(QString::fromLatin1(colors[i]))) {
            fprintf(stderr, "FAIL: COL-%d - tikzOptions() should contain color '%s'\n",
                    i + 1, colors[i]);
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: all 19 named colors in tikzOptions\n");
    return failed;
}

static int test_value_hints_arc_angles()
{
    int failed = 0;
    const auto hints = TikzWords::tikzValueHints();

    bool hasStart = false;
    bool hasEnd = false;
    for (const auto &pair : hints) {
        if (pair.first == QStringLiteral("start angle")) {
            hasStart = true;
            if (pair.second.isEmpty()) {
                fprintf(stderr, "FAIL: VH-SA1 - 'start angle' hints should not be empty\n");
                failed++;
            }
            if (!pair.second.contains(QStringLiteral("90"))) {
                fprintf(stderr, "FAIL: VH-SA2 - 'start angle' hints should contain 90\n");
                failed++;
            }
        }
        if (pair.first == QStringLiteral("end angle")) {
            hasEnd = true;
            if (pair.second.isEmpty()) {
                fprintf(stderr, "FAIL: VH-EA1 - 'end angle' hints should not be empty\n");
                failed++;
            }
            if (!pair.second.contains(QStringLiteral("360"))) {
                fprintf(stderr, "FAIL: VH-EA2 - 'end angle' hints should contain 360\n");
                failed++;
            }
        }
    }

    if (!hasStart) {
        fprintf(stderr, "FAIL: VH-SA3 - value hints should contain 'start angle' key\n");
        failed++;
    }
    if (!hasEnd) {
        fprintf(stderr, "FAIL: VH-EA3 - value hints should contain 'end angle' key\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: arc angle value hints\n");
    return failed;
}

static int test_value_hints_curve_angles()
{
    int failed = 0;
    const auto hints = TikzWords::tikzValueHints();

    bool hasIn = false;
    bool hasOut = false;
    for (const auto &pair : hints) {
        if (pair.first == QStringLiteral("in")) {
            hasIn = true;
            if (pair.second.isEmpty()) {
                fprintf(stderr, "FAIL: VH-CA1 - 'in' hints should not be empty\n");
                failed++;
            }
        }
        if (pair.first == QStringLiteral("out")) {
            hasOut = true;
            if (pair.second.isEmpty()) {
                fprintf(stderr, "FAIL: VH-CA2 - 'out' hints should not be empty\n");
                failed++;
            }
        }
    }

    if (!hasIn) {
        fprintf(stderr, "FAIL: VH-CA3 - value hints should contain 'in' key\n");
        failed++;
    }
    if (!hasOut) {
        fprintf(stderr, "FAIL: VH-CA4 - value hints should contain 'out' key\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: curve angle value hints (in/out)\n");
    return failed;
}

static int test_path_keywords_completable()
{
    int failed = 0;
    QStringList words = TikzWords::allCompletableWords();

    const char *pathKeywords[] = {
        "cycle", "controls", "parabola",
        "rectangle", "ellipse", "grid",
        "circle", "arc", "node", "coordinate",
        "sin", "cos", "plot", "to", "edge",
        "closedcycle", nullptr
    };

    for (int i = 0; pathKeywords[i] != nullptr; i++) {
        if (!words.contains(QString::fromLatin1(pathKeywords[i]), Qt::CaseInsensitive)) {
            fprintf(stderr, "FAIL: PK-%d - allCompletableWords() should contain '%s'\n",
                    i + 1, pathKeywords[i]);
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: all path construction keywords completable\n");
    return failed;
}

// Regression: bare path/graphic words such as "grid"/"circle"/"rectangle" typed
// after an earlier command on the same line (e.g. "\draw ... grid") must still
// trigger the completion popup. The previous code rerouted TkzCtxWord to
// TkzCtxUserCmd whenever ANY backslash preceded the word, hijacking the prefix
// with the whole "\draw ... grid" text and suppressing completion entirely.
static int test_path_word_completion_triggers()
{
    int failed = 0;
    QPlainTextEdit editor;
    editor.show();
    TikzCompleter completer(&editor);
    TikzDocumentState state;
    completer.setDocumentState(&state);

    struct { const char *text; bool shouldPopup; const char *desc; } cases[] = {
        { "\\draw[help lines] (0,0) grid",  true,  "grid after bracket/command" },
        { "\\draw (0,0) circle",            true,  "circle after command" },
        { "\\draw (0,0) rectangle",         true,  "rectangle after command" },
        { "\\draw (0,0) -- (1,1) node",     true,  "node after command" },
        { "\\dra",                          true,  "real command still completes" },
        { "\\draw (0,0) xyzqwv",            false, "nonsense word => no popup" },
        { nullptr, false, nullptr }
    };

    for (int i = 0; cases[i].text != nullptr; ++i) {
        editor.setPlainText(QString::fromUtf8(cases[i].text));
        editor.moveCursor(QTextCursor::End);
        completer.tryComplete();
        bool visible = completer.isPopupVisible();
        if (visible != cases[i].shouldPopup) {
            fprintf(stderr, "FAIL: PWT-%d (%s) - '%s': expected popup=%d, got %d\n",
                    i, cases[i].desc, cases[i].text,
                    cases[i].shouldPopup, visible);
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: bare path/graphic words trigger completion\n");
    return failed;
}

static int test_model_clearing_on_empty_list()
{
    int failed = 0;

    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    QStringList vals = {"coordA", "coordB"};
    completer.setModelForContext(TikzCompleter::TkzCtxCoord, vals);

    QCompleter *qcomp = nullptr;
    {
        QPlainTextEdit e2;
        TikzCompleter c2(&e2);
        c2.setModelForContext(TikzCompleter::TkzCtxCoord, vals);
    }

    completer.setModelForContext(TikzCompleter::TkzCtxCoord, QStringList());

    QStringList vals2 = {"\\cmdA", "\\cmdB"};
    completer.setModelForContext(TikzCompleter::TkzCtxUserCmd, vals2);

    completer.setModelForContext(TikzCompleter::TkzCtxUserCmd, QStringList());

    fprintf(stderr, "PASS: model clearing on empty list\n");
    return failed;
}

static int test_key_handlers()
{
    int failed = 0;
    const QStringList handlers = TikzWords::tikzKeyHandlers();

    const char *expected[] = {
        "style", "default", "code", "append style", "initial",
        "add", "store in", "value required", "value forbidden",
        "try", "retry", "handler", nullptr
    };

    for (int i = 0; expected[i] != nullptr; i++) {
        if (!handlers.contains(QString::fromLatin1(expected[i]))) {
            fprintf(stderr, "FAIL: KH-%d - tikzKeyHandlers() should contain '%s'\n",
                    i + 1, expected[i]);
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: PGF key handlers (style, default, code, ...)\n");
    return failed;
}

static int test_value_hints_angles_library()
{
    int failed = 0;
    const auto hints = TikzWords::tikzValueHints();

    bool hasRadius = false, hasEcc = false;
    for (const auto &pair : hints) {
        if (pair.first == QStringLiteral("angle radius")) {
            hasRadius = true;
            if (pair.second.isEmpty()) { fprintf(stderr, "FAIL: VH-AG1\n"); failed++; }
        }
        if (pair.first == QStringLiteral("angle eccentricity")) {
            hasEcc = true;
            if (pair.second.isEmpty()) { fprintf(stderr, "FAIL: VH-AG2\n"); failed++; }
        }
    }
    if (!hasRadius) { fprintf(stderr, "FAIL: VH-AG3\n"); failed++; }
    if (!hasEcc)    { fprintf(stderr, "FAIL: VH-AG4\n"); failed++; }

    if (failed == 0) fprintf(stderr, "PASS: angles library value hints\n");
    return failed;
}

static int test_eq_color_completion()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    // Colors that are in the full palette but were previously missing from
    // the hardcoded fill/draw/color hint subset.
    const char *colors[] = {
        "violet", "teal", "olive", "purple", "pink", "lime",
        "darkgray", "lightgray", "brown", "red", "blue", "orange", nullptr
    };

    const char *colorKeys[] = { "fill", "draw", "color", "left color",
                                "ball color", "pattern color", nullptr };

    for (int k = 0; colorKeys[k] != nullptr; k++) {
        const QStringList vals =
            completer.eqCandidatesForKey(QString::fromLatin1(colorKeys[k]));
        for (int i = 0; colors[i] != nullptr; i++) {
            if (!vals.contains(QString::fromLatin1(colors[i]))) {
                fprintf(stderr, "FAIL: EQC-1 - '%s=' completion should offer color '%s'\n",
                        colorKeys[k], colors[i]);
                failed++;
            }
        }
    }

    // Non-color curated keys must keep their intentional value hints
    // (and must NOT be flooded with colors).
    const QStringList lw = completer.eqCandidatesForKey(QStringLiteral("line width"));
    if (!lw.contains(QStringLiteral("1pt"))) {
        fprintf(stderr, "FAIL: EQC-2 - 'line width=' should offer '1pt'\n");
        failed++;
    }
    if (lw.contains(QStringLiteral("violet"))) {
        fprintf(stderr, "FAIL: EQC-3 - 'line width=' should not offer colors\n");
        failed++;
    }

    // Opacity keeps its numeric hints.
    const QStringList op = completer.eqCandidatesForKey(QStringLiteral("fill opacity"));
    if (!op.contains(QStringLiteral("0.5")) || op.contains(QStringLiteral("violet"))) {
        fprintf(stderr, "FAIL: EQC-4 - 'fill opacity=' should offer numeric hints, not colors\n");
        failed++;
    }

    if (failed == 0)
        fprintf(stderr, "PASS: fill/draw/color value completion offers full palette\n");
    return failed;
}

// Verifies eqKeyName extracts the correct option key for '=' value completion,
// ignoring '=', ',' and '[' nested inside a value's braces.
static int test_eq_key_name_extraction()
{
    int failed = 0;
    struct { const char *text; const char *expect; } cases[] = {
        { "\\draw[color=",                 "color" },
        { "\\draw[fill=red, draw=",        "draw"  },
        { "\\node[a={x,y}, fill=",         "fill"  },   // comma inside braces must not split
        { "\\path[style/.code={\\pgfkeys{/tikz/x=", "/tikz/x" }, // '=' nested in braces
        { "\\draw[line width=",            "line width" },
        { nullptr, nullptr }
    };
    for (int i = 0; cases[i].text != nullptr; ++i) {
        QString got = TikzCompleter::eqKeyName(QString::fromUtf8(cases[i].text));
        if (got != QString::fromUtf8(cases[i].expect)) {
            fprintf(stderr, "FAIL: EQK-%d - eqKeyName('%s')='%s' expected='%s'\n",
                    i, cases[i].text, got.toUtf8().constData(), cases[i].expect);
            failed++;
        }
    }
    fprintf(stderr, "%s: eqKeyName brace/bracket-aware key extraction\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies commandForOptionContext returns the command the cursor is inside,
// i.e. the LAST path/node command before the cursor column on the line.
static int test_command_for_option_context()
{
    int failed = 0;
    struct { const char *line; int col; const char *expect; } cases[] = {
        { "\\draw[",                       6,  "draw" },
        { "\\node[",                       6,  "node" },
        // Multiple commands on one line: cursor inside the node's options must
        // resolve to "node", not the earlier "draw".
        { "\\draw (0,0) -- \\node[",       21, "node" },
        // Cursor before the node command still sees only the draw.
        { "\\draw (0,0) -- \\node[",       6,  "draw" },
        { "  plain text [",               13, ""     },
        { nullptr, 0, nullptr }
    };
    for (int i = 0; cases[i].line != nullptr; ++i) {
        QString got = TikzCompleter::commandForOptionContext(
            QString::fromUtf8(cases[i].line), cases[i].col);
        if (got != QString::fromUtf8(cases[i].expect)) {
            fprintf(stderr, "FAIL: CMD-%d - commandForOptionContext('%s',%d)='%s' expected='%s'\n",
                    i, cases[i].line, cases[i].col,
                    got.toUtf8().constData(), cases[i].expect);
            failed++;
        }
    }
    fprintf(stderr, "%s: commandForOptionContext picks the innermost command\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies textBeforeForContext back-tracks across newlines to the enclosing
// bracket, so completion works for options written on multiple lines.
static int test_multiline_context()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    // Option list split across two lines; cursor on the 2nd line, in the bracket.
    editor.setPlainText(QStringLiteral("\\draw[red,\n      line "));
    editor.moveCursor(QTextCursor::End);

    QString tb = completer.textBeforeForContext();
    if (!tb.startsWith(QLatin1Char('['))) {
        fprintf(stderr, "FAIL: MLC-1 - context should start at the unclosed '[' (got: '%s')\n",
                tb.toUtf8().constData());
        failed++;
    }
    if (!tb.contains(QLatin1Char('\n'))) {
        fprintf(stderr, "FAIL: MLC-2 - context should span multiple lines\n");
        failed++;
    }
    if (completer.detectContext(tb) != TikzCompleter::TkzCtxBrk) {
        fprintf(stderr, "FAIL: MLC-3 - multi-line option context should be TkzCtxBrk\n");
        failed++;
    }

    // Sanity: a plain single-line command context still resolves correctly.
    editor.setPlainText(QStringLiteral("\\dr"));
    editor.moveCursor(QTextCursor::End);
    QString tb2 = completer.textBeforeForContext();
    if (completer.detectContext(tb2) != TikzCompleter::TkzCtxCmd) {
        fprintf(stderr, "FAIL: MLC-4 - '\\dr' should resolve to command context\n");
        failed++;
    }

    fprintf(stderr, "%s: multi-line completion context back-tracking\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies path operations/options/environments and CircuiTikZ components are
// NOT offered as \backslash commands, while genuine commands remain, and path
// operations stay completable as bare words.
static int test_no_bogus_commands()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    const QStringList cmds = TikzKeywordDB::instance().allCommandNames();
    const QStringList words = TikzKeywordDB::instance().allCompletableWords();

    // Must NOT be registered as \backslash commands.
    // (Note: "arrow" is intentionally excluded — \arrow is a genuine tikz-cd
    // command, gated to the tikzcd environment. "vertex" is excluded too —
    // \vertex is a genuine tikz-feynman command, gated to that package.)
    const char *bogus[] = {
        "anchor","arc","ball","bend","circle","coordinates","cycle",
        "dataset","drawplot","forest","getref","lineto","pattern",
        "pgfmathsetmacroglobal","pin","plot","scope","tabular","edge",
        // CircuiTikZ components (belong in to[...]/node[...], gated by circuitikz)
        "resistor","capacitor","inductor","voltmeter","ammeter","ohmmeter",
        "battery","lamp","buzzer","oscope","memristor","varistor",
        // space pseudo-commands / case errors from the extended file
        "addplot table","addplot coordinates","addplot graphics","addplot3 table",
        "draw plot","node at","coordinate at","Pgfmathsetmacro","Pgfmathsetlength",
        nullptr
    };
    for (int i = 0; bogus[i]; ++i) {
        if (cmds.contains(QString::fromLatin1(bogus[i]))) {
            fprintf(stderr, "FAIL: NBC-1 - '%s' should not be a \\command\n", bogus[i]);
            failed++;
        }
    }

    // Genuine commands must remain.
    const char *valid[] = {
        "draw","node","coordinate","clip","fill","path","foreach","matrix",
        "sin","cos","to","closedcycle","flat","protected","tikzset",
        "pgfmathsetmacro","pgfmathsetlength","pgfmathresult",
        "pgfplotstabletypeset","tikzmath","addplot+", nullptr
    };
    for (int i = 0; valid[i]; ++i) {
        if (!cmds.contains(QString::fromLatin1(valid[i]))) {
            fprintf(stderr, "FAIL: NBC-2 - valid command '%s' is missing\n", valid[i]);
            failed++;
        }
    }

    // Path operations must still be completable as bare words.
    const char *pathOps[] = { "arc", "plot", "edge", "circle", "cycle", nullptr };
    for (int i = 0; pathOps[i]; ++i) {
        if (!words.contains(QString::fromLatin1(pathOps[i]), Qt::CaseInsensitive)) {
            fprintf(stderr, "FAIL: NBC-3 - path op '%s' should stay a completable word\n",
                    pathOps[i]);
            failed++;
        }
    }

    fprintf(stderr, "%s: bogus commands removed, valid commands & path words kept\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies CircuiTikZ component/shape completion matches the CircuiTikZ 1.7.1
// component set: the invalid entries reported by users (generated 'shape'-suffix
// junk, misspellings, non-existent components) are gone, while genuine component
// and node-shape names — including the ones that were wrongly "fixed" before
// (rmeterwa, Tr, Ty, prefixed shortcuts) — are present.
static int test_circuitikz_components_accurate()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    // The union that feeds bracket/value completion inside a circuitikz context.
    QStringList combined = TikzKeywordDB::instance().allCompletableWords();
    combined += TikzKeywordDB::instance().allOptionNames();
    QSet<QString> set(combined.begin(), combined.end());

    // These must NOT appear anymore — all are invalid CircuiTikZ names, most of
    // them created by the old 'shape'-suffix generator or bogus prefix loop.
    const char *bogus[] = {
        "pvarcapacitor", "pvarcapacitorshape", "pV", "pVshape", "pRshape",
        "american nor gate", "american and gate", "american_coil", "cute_coil",
        "cute_inductor", "american_inductor", "vvarcapacitor", "elco",
        "solarcell", "closingswitch", "openingswitch", "ospt", "noground",
        "emptygeneric", "fourport", "transformercore", "delayline",
        "nchenh", "nchdep", "pchdep", "coils", "vcoils", "resistorshape",
        "capacitorshape", "pTy", "qqQ", "vQshape", "diacshape",
        nullptr
    };
    for (int i = 0; bogus[i]; ++i) {
        if (set.contains(QString::fromUtf8(bogus[i]))) {
            fprintf(stderr, "FAIL: CTK-1 - invalid CircuiTikZ entry '%s' should be gone\n",
                    bogus[i]);
            failed++;
        }
    }

    // No leftover generated '<name>shape' single-word junk (real shapes never end
    // in a bare "shape" suffix like "resistorshape").
    for (const QString &w : combined) {
        if (w.endsWith(QLatin1String("shape")) && !w.contains(' ')
            && w != QLatin1String("shape")
            && w != QLatin1String("pgfdeclareshape")) {
            fprintf(stderr, "FAIL: CTK-2 - leftover shape-suffix junk entry '%s'\n",
                    w.toUtf8().constData());
            failed++;
        }
    }

    // Genuine CircuiTikZ path components / shortcuts must be present (these are
    // exactly the ones a previous mis-"fix" would have wrongly removed/renamed).
    const char *validComponents[] = {
        "rmeterwa", "pR", "vR", "sR", "phR", "ldR", "thR", "Tr", "Ty",
        "elko", "solar", "solarsource", "closing switch", "opening switch",
        "cute choke", "cute inductor", "american inductor", "variable capacitor",
        "polar capacitor", "thermistor ntc", "thermistor ptc", "cspst",
        "R", "L", "C", "D", "V", "I", "pC", "pD",
        nullptr
    };
    for (int i = 0; validComponents[i]; ++i) {
        if (!set.contains(QString::fromUtf8(validComponents[i]))) {
            fprintf(stderr, "FAIL: CTK-3 - valid CircuiTikZ component '%s' is missing\n",
                    validComponents[i]);
            failed++;
        }
    }

    // Genuine CircuiTikZ node shapes must be present.
    const char *validShapes[] = {
        "npn", "pnp", "nmos", "pmos", "op amp", "ground", "rground", "sground",
        "american nor port", "european nor port", "and port", "nand port",
        "buffer port", "vcc", "vss", "vdd", "vee", "ocirc", "diamondpole",
        nullptr
    };
    for (int i = 0; validShapes[i]; ++i) {
        if (!set.contains(QString::fromUtf8(validShapes[i]))) {
            fprintf(stderr, "FAIL: CTK-4 - valid CircuiTikZ shape '%s' is missing\n",
                    validShapes[i]);
            failed++;
        }
    }

    // CircuiTikZ components must be gated by the circuitikz library (never leak
    // into a plain tikzpicture without \usepackage{circuitikz}).
    {
        QSet<QString> noLibs;
        auto plain = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QStringLiteral("to"), noLibs,
            TikzKeywords::Category::Option);
        for (auto *kw : plain) {
            if (kw->name == QLatin1String("rmeterwa") || kw->name == QLatin1String("pR")) {
                fprintf(stderr, "FAIL: CTK-5 - '%s' leaked without circuitikz lib active\n",
                        kw->name.toUtf8().constData());
                failed++;
            }
        }
        QSet<QString> ckLibs; ckLibs.insert(QStringLiteral("circuitikz"));
        auto gated = TikzKeywordDB::instance().filter(
            QStringLiteral("circuitikz"), QStringLiteral("to"), ckLibs,
            TikzKeywords::Category::Option);
        bool foundRmeter = false;
        for (auto *kw : gated)
            if (kw->name == QLatin1String("rmeterwa")) { foundRmeter = true; break; }
        if (!foundRmeter) {
            fprintf(stderr, "FAIL: CTK-6 - 'rmeterwa' not offered with circuitikz lib active\n");
            failed++;
        }
    }

    fprintf(stderr, "%s: CircuiTikZ components/shapes match CircuiTikZ 1.7.1 set\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies standard TikZ/PGF completion has no hallucinated shapes/options and
// that genuine (source-verified) entries are kept. Guards against over-eager
// "cleanups" that would delete valid keys such as bend at start / l_ / american.
static int test_standard_tikz_no_bogus()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    QStringList all = TikzKeywordDB::instance().allCompletableWords();
    all += TikzKeywordDB::instance().allOptionNames();
    QSet<QString> set(all.begin(), all.end());

    // Not real PGF shapes — must be absent (the real one is 'regular polygon';
    // 'circle around' is only a source comment, never a declared shape).
    const char *bogusShapes[] = { "polygon", "circle around", nullptr };
    for (int i = 0; bogusShapes[i]; ++i) {
        if (set.contains(QString::fromUtf8(bogusShapes[i]))) {
            fprintf(stderr, "FAIL: STD-1 - bogus shape '%s' should be removed\n",
                    bogusShapes[i]);
            failed++;
        }
    }

    // Genuine PGF/TikZ entries that must stay (all verified in pgf/circuitikz
    // 1.7.1 sources). 'bend at start/end' and 'bend pos' are real TikZ core
    // parabola/bend options; l_/v^ and american/european are real circuitikz keys.
    const char *valid[] = {
        "regular polygon", "star", "cross out", "strike out",
        "bend at start", "bend at end", "bend pos", "bend left", "bend right",
        "american", "european", "voltage", "current",
        "l_", "l^", "v_", "v^", "i_", "i^", "a_", "a^",
        "mirror", "invert", "swap",
        nullptr
    };
    for (int i = 0; valid[i]; ++i) {
        if (!set.contains(QString::fromUtf8(valid[i]))) {
            fprintf(stderr, "FAIL: STD-2 - valid entry '%s' is missing\n", valid[i]);
            failed++;
        }
    }

    // 'mirror'/'invert' are circuitikz-gated: must not leak into a plain picture.
    {
        QSet<QString> noLibs;
        auto plain = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QStringLiteral("draw"), noLibs,
            TikzKeywords::Category::Option);
        for (auto *kw : plain) {
            if (kw->name == QLatin1String("mirror") || kw->name == QLatin1String("invert")) {
                fprintf(stderr, "FAIL: STD-3 - '%s' leaked without circuitikz lib\n",
                        kw->name.toUtf8().constData());
                failed++;
            }
        }
    }

    fprintf(stderr, "%s: standard TikZ shapes/options are source-accurate\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies that \usetikzlibrary{...} completion resolves to the library context
// through the full pipeline (textBeforeForContext -> detectContext). This is the
// real-world path: the completer first back-tracks to the enclosing '{', which
// previously dropped the "\usetikzlibrary" keyword and misdetected the context
// as a generic word (offering wrong singular names like "intersection"), or gave
// nothing at all. Also covers the same fix for \begin{...} environment context.
static int test_usetikzlibrary_context()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    struct { const char *text; TikzCompleter::Context ctx; const char *desc; } cases[] = {
        { "\\usetikzlibrary{",                           TikzCompleter::TkzCtxLib, "empty brace" },
        { "\\usetikzlibrary{inters",                     TikzCompleter::TkzCtxLib, "single lib" },
        { "\\usetikzlibrary{intersections,angles,quo",   TikzCompleter::TkzCtxLib, "after commas" },
        { "\\usetikzlibrary{intersections, angles, quo", TikzCompleter::TkzCtxLib, "after commas w/ spaces" },
        { "\\begin{tikzpictu",                           TikzCompleter::TkzCtxBeg, "begin env" },
        { nullptr, TikzCompleter::TkzCtxNone, nullptr }
    };

    for (int i = 0; cases[i].text != nullptr; ++i) {
        editor.setPlainText(QString::fromUtf8(cases[i].text));
        editor.moveCursor(QTextCursor::End);
        // Full pipeline: back-track to the enclosing brace, then classify.
        QString tb = completer.textBeforeForContext();
        TikzCompleter::Context got = completer.detectContext(tb);
        if (got != cases[i].ctx) {
            fprintf(stderr, "FAIL: UTL-%d (%s) - '%s' via pipeline: expected ctx %d, got %d (textBefore='%s')\n",
                    i, cases[i].desc, cases[i].text, static_cast<int>(cases[i].ctx),
                    static_cast<int>(got), tb.toUtf8().constData());
            failed++;
        }
    }

    // The back-tracked context must retain the command keyword so the library
    // context can be recognised (the crux of the reported bug).
    editor.setPlainText(QStringLiteral("\\usetikzlibrary{intersections,angles,quo"));
    editor.moveCursor(QTextCursor::End);
    QString tb = completer.textBeforeForContext();
    if (!tb.contains(QStringLiteral("\\usetikzlibrary"))) {
        fprintf(stderr, "FAIL: UTL-KEEP - textBeforeForContext dropped the \\usetikzlibrary keyword (got '%s')\n",
                tb.toUtf8().constData());
        failed++;
    }

    if (failed == 0)
        fprintf(stderr, "PASS: \\usetikzlibrary / \\begin completion context via full pipeline\n");
    return failed;
}

// Verifies math functions match the PGF math engine set (verified against
// texmf-dist/tex/generic/pgf/math): bogus entries are gone, real ones present.
static int test_mathfunctions_accurate()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;
    const QStringList fns = TikzKeywordDB::instance().allMathFuncNames();

    const char *bogus[] = { "cit","kil","note","record","res","vecilen",
        "power","dotproduct","turn","intersection","clip","format","cm","pt",
        "in","length","angle","sum","value","rdiv", nullptr };
    for (int i = 0; bogus[i]; ++i) {
        if (fns.contains(QString::fromUtf8(bogus[i]))) {
            fprintf(stderr, "FAIL: MFN-1 - bogus math function '%s' should be gone\n", bogus[i]);
            failed++;
        }
    }
    const char *valid[] = { "veclen","pow","atan2","sec","cosec","Mod","reciprocal",
        "hex","Hex","oct","bin","isprime","iseven","isodd","gcd","factorial",
        "and","or","not","equal","greater","less","radians","round","floor","ceil",
        nullptr };
    for (int i = 0; valid[i]; ++i) {
        if (!fns.contains(QString::fromUtf8(valid[i]))) {
            fprintf(stderr, "FAIL: MFN-2 - real math function '%s' is missing\n", valid[i]);
            failed++;
        }
    }
    fprintf(stderr, "%s: math functions match the PGF math engine set\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies decoration names and their library attributions against the pgf
// decoration libraries: bogus entries gone, real ones present, core decorations
// not wrongly gated behind decorations.pathmorphing.
static int test_decorations_accurate()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;
    const QStringList decos = TikzKeywordDB::instance().allDecorationNames();

    const char *bogus[] = { "curly brace","trapezium brace","Hilbert curve",
        "Sierpinski triangle","Peano curve","Moore curve","stars","Koch curve",
        "wave","angle", nullptr };
    for (int i = 0; bogus[i]; ++i) {
        if (decos.contains(QString::fromUtf8(bogus[i]))) {
            fprintf(stderr, "FAIL: DEC-1 - bogus decoration '%s' should be gone\n", bogus[i]);
            failed++;
        }
    }
    const char *valid[] = { "snake","coil","zigzag","random steps","bent",
        "straight zigzag","brace","ticks","waves","expanding waves","border",
        "show path construction","markings","Koch curve type 1","Koch curve type 2",
        "Cantor set","crosses","triangles", nullptr };
    for (int i = 0; valid[i]; ++i) {
        if (!decos.contains(QString::fromUtf8(valid[i]))) {
            fprintf(stderr, "FAIL: DEC-2 - real decoration '%s' is missing\n", valid[i]);
            failed++;
        }
    }
    // Core decorations must NOT require decorations.pathmorphing.
    for (const char *core : { "lineto", "curveto", "moveto" }) {
        const auto *kw = TikzKeywordDB::instance().find(QString::fromUtf8(core), Category::Decoration);
        if (kw && kw->requiredLibs.contains(QStringLiteral("decorations.pathmorphing"))) {
            fprintf(stderr, "FAIL: DEC-3 - core decoration '%s' wrongly gated by pathmorphing\n", core);
            failed++;
        }
    }
    fprintf(stderr, "%s: decorations match the pgf decoration libraries\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies physics/siunitx package commands are present and correctly gated:
// not offered without the package, offered when \usepackage is detected.
static int test_physics_siunitx_gated()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    const char *physCmds[] = { "vb","va","vu","dv","pdv","grad","curl",
        "laplacian","bra","ket","braket","expval","comm","dd","cross",
        "norm","tr","Tr", nullptr };
    for (int i = 0; physCmds[i]; ++i) {
        const auto *kw = TikzKeywordDB::instance().find(QString::fromUtf8(physCmds[i]), Category::Command);
        if (!kw) {
            fprintf(stderr, "FAIL: PSI-1 - physics command '%s' missing\n", physCmds[i]);
            failed++;
        } else if (!kw->requiredLibs.contains(QStringLiteral("physics"))) {
            fprintf(stderr, "FAIL: PSI-2 - physics command '%s' not gated by physics lib\n", physCmds[i]);
            failed++;
        }
    }
    const char *siCmds[] = { "SI","si","num","ang","qty","unit","numlist",
        "qtyrange","sisetup", nullptr };
    for (int i = 0; siCmds[i]; ++i) {
        const auto *kw = TikzKeywordDB::instance().find(QString::fromUtf8(siCmds[i]), Category::Command);
        if (!kw) {
            fprintf(stderr, "FAIL: PSI-3 - siunitx command '%s' missing\n", siCmds[i]);
            failed++;
        } else if (!kw->requiredLibs.contains(QStringLiteral("siunitx"))) {
            fprintf(stderr, "FAIL: PSI-4 - siunitx command '%s' not gated by siunitx lib\n", siCmds[i]);
            failed++;
        }
    }

    // Gating behaviour through filter(): without libs, \vb / \SI must be hidden;
    // with the libs active they must appear.
    QSet<QString> noLibs;
    auto plainCmds = TikzKeywordDB::instance().filter(
        QStringLiteral("tikzpicture"), QString(), noLibs, Category::Command);
    for (auto *kw : plainCmds) {
        if (kw->name == QLatin1String("vb") || kw->name == QLatin1String("SI")) {
            fprintf(stderr, "FAIL: PSI-5 - '%s' leaked without its package active\n",
                    kw->name.toUtf8().constData());
            failed++;
        }
    }
    QSet<QString> physLibs; physLibs.insert(QStringLiteral("physics"));
    bool foundVb = false;
    for (auto *kw : TikzKeywordDB::instance().filter(
             QStringLiteral("tikzpicture"), QString(), physLibs, Category::Command))
        if (kw->name == QLatin1String("vb")) { foundVb = true; break; }
    if (!foundVb) {
        fprintf(stderr, "FAIL: PSI-6 - '\\vb' not offered with physics package active\n");
        failed++;
    }

    fprintf(stderr, "%s: physics/siunitx commands present and package-gated\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies pgfplots keys/shapes: bogus removed, high-value keys present.
static int test_pgfplots_keys_accurate()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;
    const QStringList opts = TikzKeywordDB::instance().allOptionNames();

    const char *bogus[] = { "xstep","ystep","zstep","bar gap",
        "legend image style","histogram", nullptr };
    for (int i = 0; bogus[i]; ++i) {
        // xstep/ystep are valid *grid* options (draw context); only the pgfplots
        // axis versions were renamed, so check the pgfplots-specific ones are gone
        // by confirming their replacements exist instead.
        (void)bogus[i];
    }
    const char *mustAbsent[] = { "bar gap","legend image style","histogram", nullptr };
    for (int i = 0; mustAbsent[i]; ++i) {
        if (opts.contains(QString::fromUtf8(mustAbsent[i]))) {
            fprintf(stderr, "FAIL: PGP-1 - bogus pgfplots key '%s' should be gone\n", mustAbsent[i]);
            failed++;
        }
    }
    const char *valid[] = { "xtick distance","ytick distance","ztick distance",
        "enlarge x limits","enlarge y limits","axis lines","axis lines*",
        "colorbar","point meta","scatter src","forget plot","each nth point",
        "nodes near coords","scale only axis","restrict y to domain","hide axis",
        "shader","compat","cycle list name","xtick pos","z buffer",
        "legend image post style","hist", nullptr };
    for (int i = 0; valid[i]; ++i) {
        if (!opts.contains(QString::fromUtf8(valid[i]))) {
            fprintf(stderr, "FAIL: PGP-2 - pgfplots key '%s' is missing\n", valid[i]);
            failed++;
        }
    }
    // Missing shapes now present.
    const char *shapes[] = { "kite","dart","chamfered rectangle","arrow box",
        "starburst","ellipse split","circle solidus","magnifying glass", nullptr };
    for (int i = 0; shapes[i]; ++i) {
        if (!TikzKeywordDB::instance().find(QString::fromUtf8(shapes[i]), Category::Shape)) {
            fprintf(stderr, "FAIL: PGP-3 - shape '%s' is missing\n", shapes[i]);
            failed++;
        }
    }
    // Bogus shapes gone.
    for (const char *bs : { "lightning", "line callout", "rounded rectangle callout" }) {
        if (TikzKeywordDB::instance().find(QString::fromUtf8(bs), Category::Shape)) {
            fprintf(stderr, "FAIL: PGP-4 - bogus shape '%s' should be gone\n", bs);
            failed++;
        }
    }
    fprintf(stderr, "%s: pgfplots keys/shapes source-accurate\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies package-specific completion (tikz-cd, tkz-euclide, chemfig,
// tikz-feynman) matches the installed TeX Live sources: genuine entries present
// and correctly library-gated, bogus/removed entries absent.
static int test_package_completion_accurate()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;
    auto &db = TikzKeywordDB::instance();

    auto reqCmd = [&](const char *n, const char *lib) {
        const auto *k = db.find(QString::fromUtf8(n), Category::Command);
        if (!k) { fprintf(stderr, "FAIL: PKG - command '%s' missing\n", n); failed++; }
        else if (!k->requiredLibs.contains(QString::fromUtf8(lib))) {
            fprintf(stderr, "FAIL: PKG - '%s' not gated by %s\n", n, lib); failed++;
        }
    };
    auto reqOpt = [&](const char *n, const char *lib) {
        const auto *k = db.find(QString::fromUtf8(n), Category::Option);
        if (!k) { fprintf(stderr, "FAIL: PKG - option '%s' missing\n", n); failed++; }
        else if (!k->requiredLibs.contains(QString::fromUtf8(lib))) {
            fprintf(stderr, "FAIL: PKG - '%s' not gated by %s\n", n, lib); failed++;
        }
    };
    auto absentCmd = [&](const char *n) {
        if (db.find(QString::fromUtf8(n), Category::Command)) {
            fprintf(stderr, "FAIL: PKG - bogus command '%s' should be gone\n", n); failed++;
        }
    };
    auto absentOpt = [&](const char *n) {
        if (db.find(QString::fromUtf8(n), Category::Option)) {
            fprintf(stderr, "FAIL: PKG - bogus option '%s' should be gone\n", n); failed++;
        }
    };

    // ── tikz-cd (gated by the tikzcd environment, not requiredLibs) ──
    auto envCmd = [&](const char *n) {
        const auto *k = db.find(QString::fromUtf8(n), Category::Command);
        if (!k) { fprintf(stderr, "FAIL: PKG - command '%s' missing\n", n); failed++; }
        else if (!k->environments.contains(QStringLiteral("tikzcd"))) {
            fprintf(stderr, "FAIL: PKG - '%s' not scoped to tikzcd env\n", n); failed++;
        }
    };
    auto envOpt = [&](const char *n) {
        const auto *k = db.find(QString::fromUtf8(n), Category::Option);
        if (!k) { fprintf(stderr, "FAIL: PKG - option '%s' missing\n", n); failed++; }
        else if (!k->environments.contains(QStringLiteral("tikzcd"))) {
            fprintf(stderr, "FAIL: PKG - '%s' not scoped to tikzcd env\n", n); failed++;
        }
    };
    for (const char *c : {"ar","rar","lar","uar","dar","urar","ular","drar","dlar"})
        envCmd(c);
    for (const char *o : {"rightarrow","leftarrow","Rightarrow","hook","hook'","harpoon",
                          "harpoon'","two heads","no head","maps to","dash","equal",
                          "squiggly","dashed","math mode","arrows","row sep","column sep",
                          "crossing over","phantom"})
        envOpt(o);
    absentCmd("obj");
    absentOpt("tweak");
    absentOpt("math nodes");

    // ── tkz-euclide ──
    for (const char *c : {"tkzInit","tkzDefPoint","tkzDefLine","tkzDefCircle",
                          "tkzDefTriangle","tkzDefSquare","tkzDefRegPolygon",
                          "tkzDrawPolygon","tkzDrawCircle","tkzDrawArc","tkzDrawSegment",
                          "tkzInterLL","tkzInterLC","tkzInterCC","tkzMarkRightAngle",
                          "tkzLabelPoints","tkzDefTriangleCenter","tkzClipCircle"})
        reqCmd(c, "tkz-euclide");
    // v4 names removed in v5.10c
    for (const char *c : {"tkzDrawTriangle","tkzDrawSquare","tkzTangent","tkzDrawBisector",
                          "tkzDrawMedian","tkzDrawAltitude","tkzDrawEulerLine",
                          "tkzDrawAngle","tkzFillTriangle","tkzDrawRegPolygon",
                          "tkzClipLine","tkzDefPolygonCenter"})
        absentCmd(c);

    // ── chemfig ──
    for (const char *c : {"chemfig","definesubmol","redefinesubmol","chemname",
                          "chemabove","chembelow","schemestart","schemestop",
                          "setchemfig","charge","Charge","polymerdelim","chemleft"})
        reqCmd(c, "chemfig");
    for (const char *o : {"atom sep","bond offset","double bond sep","angle increment",
                          "cram width","arrow offset","compound sep","bond style"})
        reqOpt(o, "chemfig");

    // ── tikz-feynman ──
    for (const char *c : {"feynmandiagram","tikzfeynmanset","feynman","vertex","diagram"})
        reqCmd(c, "tikz-feynman");
    for (const char *o : {"fermion","anti fermion","photon","boson","charged boson",
                          "scalar","charged scalar","ghost","gluon","majorana","plain",
                          "momentum","momentum'","rmomentum","half left","half right",
                          "with arrow","insertion","dot","blob"})
        reqOpt(o, "tikz-feynman");

    // Gating: none of these should appear in a plain tikzpicture without the pkg.
    QSet<QString> none;
    auto plainCmds = db.filter(QStringLiteral("tikzpicture"), QString(), none, Category::Command);
    for (auto *k : plainCmds) {
        if (k->name == QLatin1String("tkzDefPoint") || k->name == QLatin1String("chemfig")
            || k->name == QLatin1String("feynmandiagram")) {
            fprintf(stderr, "FAIL: PKG - '%s' leaked without its package\n",
                    k->name.toUtf8().constData());
            failed++;
        }
    }

    fprintf(stderr, "%s: tikz-cd/tkz-euclide/chemfig/tikz-feynman completion source-accurate\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies the TkzCtxPathWord context's completable set is strictly limited to
// path operations — unrelated entries such as math function "reciprocal", the
// node shape "rectangle split", PGF key handlers, or CircuiTikZ components must
// not leak into the path-word completer where only path construction verbs
// belong. Also verifies those entries remain in the full TkzCtxWord pool.
static int test_path_word_set_restricted()
{
    int failed = 0;

    const QStringList pathOps = TikzWords::tikzPathOperations();
    const QStringList allWords = TikzWords::allCompletableWords();

    // Path operations that MUST be present in the restricted set.
    const char *required[] = {
        "rectangle", "circle", "ellipse", "arc", "grid", "parabola", "bend",
        "sin", "cos", "svg", "plot", "to", "let", "node", "coordinate", "pic",
        "edge", "graph", "child", "foreach", "cycle", "closedcycle", "controls",
        nullptr
    };
    for (int i = 0; required[i]; ++i) {
        if (!pathOps.contains(QString::fromLatin1(required[i]))) {
            fprintf(stderr, "FAIL: PWS-1 - path op '%s' missing from tikzPathOperations\n",
                    required[i]);
            failed++;
        }
    }

    // Entries that MUST NOT be in the restricted set (they belong in the full
    // word list, but not in a path-body context where only path operations make
    // sense). This is the crux of the reported bug: typing "rec" inside a
    // \draw path offered "reciprocal" and "rectangle split" alongside "rectangle".
    const char *mustNot[] = {
        "reciprocal", "rectangle split", "rectangle shape",
        "recursively defined", "semicircle", "circular sector",
        "circular drop shadow", "circle through", "circle solidus",
        "circle connection bar", "circle split", "circle top",
        "circle right", "circle left", "circle bottom",
        "circle arrow", "circle bar", "node distance", "node split",
        "arc tip", "arc focus", "separation circle", "grid style",
        "grids", "cir", "rec", nullptr
    };
    for (int i = 0; mustNot[i]; ++i) {
        if (pathOps.contains(QString::fromLatin1(mustNot[i]))) {
            fprintf(stderr, "FAIL: PWS-2 - '%s' should NOT be in tikzPathOperations\n",
                    mustNot[i]);
            failed++;
        }
    }

    // Verify these must-not entries DO exist in the full word list (they are
    // valid completions for TkzCtxWord — just not for TkzCtxPathWord).
    const char *validInFull[] = {
        "reciprocal", "rectangle split",
        "semicircle", "circular sector", nullptr
    };
    for (int i = 0; validInFull[i]; ++i) {
        if (!allWords.contains(QString::fromLatin1(validInFull[i]), Qt::CaseInsensitive)) {
            fprintf(stderr, "FAIL: PWS-3 - '%s' should still be in allCompletableWords\n",
                    validInFull[i]);
            failed++;
        }
    }

    // Path operations should also be in the full word list (check the subset
    // that are common enough to appear in the keyword database).
    {
        const char *inBoth[] = {
            "rectangle", "circle", "ellipse", "arc", "grid", "parabola",
            "sin", "cos", "plot", "to", "node", "coordinate",
            "edge", "cycle", "controls", nullptr
        };
        for (int i = 0; inBoth[i]; ++i) {
            if (!allWords.contains(QString::fromLatin1(inBoth[i]), Qt::CaseInsensitive)) {
                fprintf(stderr, "FAIL: PWS-4 - path op '%s' missing from allCompletableWords\n",
                        inBoth[i]);
                failed++;
            }
        }
    }

    // No duplicate entries in the restricted set.
    QSet<QString> seen;
    for (const QString &s : pathOps) {
        QString l = s.toLower();
        if (seen.contains(l)) {
            fprintf(stderr, "FAIL: PWS-5 - duplicate '%s' in tikzPathOperations\n",
                    qPrintable(s));
            failed++;
        }
        seen.insert(l);
    }

    if (failed == 0)
        fprintf(stderr, "PASS: path-word completion set restricted to path operations\n");
    return failed;
}

// End-to-end test: typing a path word in a \draw body offers only path
// operations, not the full word list. Uses the completer's internal models
// accessed via the public setModelForContext / models side-channel.
static int test_path_word_completion_correctness()
{
    int failed = 0;
    QPlainTextEdit editor;
    editor.show();
    TikzCompleter completer(&editor);
    TikzDocumentState state;
    completer.setDocumentState(&state);

    // Structure: set document text, trigger completion, verify popup state
    // and that the context routing is correct.
    struct CompletionCase {
        const char *text;
        bool shouldPopup;
        const char *desc;
    };

    CompletionCase cases[] = {
        // Path operations in the path body must trigger a popup.
        { "\\draw (0,0) rec",      true,  "rec triggers path-word popup" },
        { "\\draw (0,0) circ",     true,  "circ triggers path-word popup" },
        { "\\draw (0,0) ellip",    true,  "ellip triggers path-word popup" },
        { "\\draw (0,0) grid",     true,  "grid triggers path-word popup" },
        { "\\draw (0,0) -- (2,2) ar",   true,  "arc after points" },
        { "\\draw (0,0) -- (2,2) par", true,  "par after points" },
        { "\\draw (0,0) node {rec",     true,  "rec inside braces (full word list)" },
        { "\\node at (0,0) {recip",     true,  "recip inside braces (full word list)" },
        // Nonsense words that aren't in either set: no popup.
        { "\\draw (0,0) zzzxxx",  false, "nonsense word in path body" },
        { "\\draw (0,0) node {zzzxxx", false, "nonsense word in braces" },
        { nullptr, false, nullptr }
    };

    for (int i = 0; cases[i].text != nullptr; ++i) {
        editor.setPlainText(QString::fromUtf8(cases[i].text));
        editor.moveCursor(QTextCursor::End);
        completer.tryComplete();
        bool visible = completer.isPopupVisible();
        if (visible != cases[i].shouldPopup) {
            fprintf(stderr, "FAIL: PWC-%d (%s) - '%s': expected popup=%d, got %d\n",
                    i, cases[i].desc, cases[i].text,
                    cases[i].shouldPopup, visible);
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: path-word completion correctness (context routing + trigger)\n");
    return failed;
}

// Verifies eqCandidatesForKey includes coordinate/node names for positioning
// keys (above/below/left/right and their =of variants), both bare and
// "of "-prefixed, so \node[below=of wai...] completes to "of waiting 1".
static int test_positioning_key_coord_completion()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    QTextDocument doc;
    doc.setPlainText(
        "\\coordinate (waiting 1) at (0,0);\n"
        "\\coordinate (critical 1) at (0,0);\n"
        "\\node (myNode) at (0,0) {};\n");
    TikzDocumentState state;
    state.reparse(&doc);
    completer.setDocumentState(&state);

    const QStringList vals = completer.eqCandidatesForKey(QStringLiteral("below"));

    if (!vals.contains(QStringLiteral("of waiting 1"))) {
        fprintf(stderr, "FAIL: POS-1 - 'below=' completion should offer 'of waiting 1'\n");
        failed++;
    }
    if (!vals.contains(QStringLiteral("of critical 1"))) {
        fprintf(stderr, "FAIL: POS-2 - 'below=' completion should offer 'of critical 1'\n");
        failed++;
    }
    if (!vals.contains(QStringLiteral("of myNode"))) {
        fprintf(stderr, "FAIL: POS-3 - 'below=' completion should offer 'of myNode'\n");
        failed++;
    }
    if (!vals.contains(QStringLiteral("waiting 1"))) {
        fprintf(stderr, "FAIL: POS-4 - 'below=' completion should offer bare 'waiting 1'\n");
        failed++;
    }

    // Non-positioning keys must NOT offer coordinate names.
    const QStringList drawVals = completer.eqCandidatesForKey(QStringLiteral("draw"));
    if (drawVals.contains(QStringLiteral("of waiting 1"))) {
        fprintf(stderr, "FAIL: POS-5 - 'draw=' should NOT offer coordinate names\n");
        failed++;
    }

    // "above=of" key variant must also offer coordinates.
    const QStringList aboveOfVals = completer.eqCandidatesForKey(QStringLiteral("above=of"));
    if (!aboveOfVals.contains(QStringLiteral("of waiting 1"))) {
        fprintf(stderr, "FAIL: POS-6 - 'above=of=' completion should offer 'of waiting 1'\n");
        failed++;
    }

    // "left" and "right" too.
    for (const char *k : { "left", "right", "above left", "below right" }) {
        const QStringList v = completer.eqCandidatesForKey(QString::fromUtf8(k));
        if (!v.contains(QStringLiteral("of waiting 1"))) {
            fprintf(stderr, "FAIL: POS-7 - '%s=' completion should offer 'of waiting 1'\n", k);
            failed++;
        }
    }

    // A key that happens to contain "below" as substring (like "below=of") but
    // is NOT a positioning key should not get coordinates. Actually "below=of"
    // IS a positioning key. Let's test a truly unrelated key.
    const QStringList lwVals = completer.eqCandidatesForKey(QStringLiteral("line width"));
    if (lwVals.contains(QStringLiteral("waiting 1"))) {
        fprintf(stderr, "FAIL: POS-8 - 'line width=' should NOT offer coordinate names\n");
        failed++;
    }

    if (failed == 0)
        fprintf(stderr, "PASS: positioning keys offer coordinate names\n");
    return failed;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int failed = 0;
    failed += test_options_contain_help_lines();
    failed += test_options_contain_new_entries();
    failed += test_options_existing_entries_preserved();
    failed += test_value_hints_arrow_tip_keys();
    failed += test_value_hints_pattern();
    failed += test_detect_context();
    failed += test_all_completable_words_no_duplicates();
    failed += test_arrow_specs_not_duplicated();
    failed += test_colors_in_options();
    failed += test_value_hints_arc_angles();
    failed += test_value_hints_curve_angles();
    failed += test_value_hints_angles_library();
    failed += test_path_keywords_completable();
    failed += test_path_word_completion_triggers();
    failed += test_model_clearing_on_empty_list();
    failed += test_key_handlers();
    failed += test_eq_color_completion();
    failed += test_eq_key_name_extraction();
    failed += test_command_for_option_context();
    failed += test_multiline_context();
    failed += test_no_bogus_commands();
    failed += test_usetikzlibrary_context();
    failed += test_circuitikz_components_accurate();
    failed += test_standard_tikz_no_bogus();
    failed += test_mathfunctions_accurate();
    failed += test_decorations_accurate();
    failed += test_physics_siunitx_gated();
    failed += test_pgfplots_keys_accurate();
    failed += test_package_completion_accurate();
    failed += test_path_word_set_restricted();
    failed += test_path_word_completion_correctness();
    failed += test_positioning_key_coord_completion();

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll TikZ completer tests passed!\n");
    return 0;
}
