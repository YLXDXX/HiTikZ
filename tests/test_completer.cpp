#include <QApplication>
#include <QPlainTextEdit>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QSet>
#include <cstdio>
#include "tikz_words.h"
#include "tikz_completer.h"

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
        {"zstep", "pgfplots zstep"},
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
        {"information text", "information text style"},
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
        {QStringLiteral("\\draw[help lines] (0,0) gr"), TikzCompleter::TkzCtxWord, "keyword after bracket"},
        {QStringLiteral("\\node[draw] at (0.5,"), TikzCompleter::TkzCtxNone, "coordinates"},
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
    failed += test_model_clearing_on_empty_list();
    failed += test_key_handlers();
    failed += test_eq_color_completion();
    failed += test_eq_key_name_extraction();
    failed += test_command_for_option_context();
    failed += test_multiline_context();

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll TikZ completer tests passed!\n");
    return 0;
}
