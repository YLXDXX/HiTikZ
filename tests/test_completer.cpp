#include <QApplication>
#include <QPlainTextEdit>
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QSet>
#include <cstdio>
#include <QKeyEvent>
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
        {QStringLiteral("\\end{tikzpictu"), TikzCompleter::TkzCtxEnd, "inside end"},
        {QStringLiteral("\\end{scop"), TikzCompleter::TkzCtxEnd, "end with partial name"},
        {QStringLiteral("\\end{"), TikzCompleter::TkzCtxEnd, "empty end brace"},
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
        // Eq-value context triggered by = as word boundary (inside braces
        // where no bracket exists), e.g. .style={->,>=s} should complete
        // arrow names just like [>=s] does.
        {QStringLiteral("{->,>=s"), TikzCompleter::TkzCtxEq, ">= value inside braces"},
        {QStringLiteral("{fill=re"), TikzCompleter::TkzCtxEq, "= value inside braces (color)"},
        // Word after = that is long enough for generic word detection must
        // still route to TkzCtxEq, not TkzCtxWord (no space between = and value).
        {QStringLiteral("{>=st"), TikzCompleter::TkzCtxEq, "= value >=2 chars inside braces (no space)"},
        {QStringLiteral("\\node[draw] at (0.5,"), TikzCompleter::TkzCtxNone, "coordinates"},
        {QStringLiteral("\\usetikzlibrary{"), TikzCompleter::TkzCtxLib, "empty usetikzlibrary brace"},
        {QStringLiteral("\\usetikzlibrary{inters"), TikzCompleter::TkzCtxLib, "single library typing"},
        {QStringLiteral("\\usetikzlibrary{intersections,angles,quo"), TikzCompleter::TkzCtxLib, "library after commas"},
        // Options that embed a bracketed value inside braces (e.g. arrow tip
        // specs like >={Stealth[round]}) must NOT break subsequent completion in
        // the SAME '[...]' option list. The ']' inside the braces is nested and
        // must be ignored when locating the enclosing '['.
        {QStringLiteral("[post,>={Stealth[round]},"), TikzCompleter::TkzCtxBrk, "new key after bracketed-brace value"},
        {QStringLiteral("[post,>={Stealth[round]},semi"), TikzCompleter::TkzCtxBrk, "typing key after bracketed-brace value"},
        {QStringLiteral("[auto,pre/.style={<-,>={Stealth[round]},semithick},"), TikzCompleter::TkzCtxBrk, "new key after style with bracketed arrow"},
        {QStringLiteral("[auto,pre/.style={<-,>={Stealth[round]},semithick},th"), TikzCompleter::TkzCtxBrk, "typing key after style with bracketed arrow"},
        {QStringLiteral("[>={Stealth[round]},fill="), TikzCompleter::TkzCtxEq, "value context after bracketed-brace value"},
        // A ']' nested in braces must not be mistaken for the closer of an
        // earlier bracket group when the cursor is genuinely inside brackets.
        {QStringLiteral("[a={x]y},b"), TikzCompleter::TkzCtxBrk, "key after value containing a stray-looking ]"},
        // Coordinate-system usage "(name cs:key,...)": after the "cs:" marker
        // the tokens are option keys, not coordinate names.
        {QStringLiteral("\\draw (0,0,0) -- (xyz cylindrical cs:"), TikzCompleter::TkzCtxCoordSysKey, "cs: marker -> coord-sys key"},
        {QStringLiteral("\\draw (xyz cylindrical cs:an"), TikzCompleter::TkzCtxCoordSysKey, "typing key after cs:"},
        {QStringLiteral("\\draw (xyz spherical cs:radius=1,long"), TikzCompleter::TkzCtxCoordSysKey, "second key after comma"},
        // The value part after '=' inside cs: must not offer key completion.
        {QStringLiteral("\\draw (xyz cylindrical cs:z=1"), TikzCompleter::TkzCtxNone, "value after cs: key = is silent"},
        // Regression: a completed value containing closed paren pairs (e.g.
        // "first line={(A)--(B)},") used to hide the governing '(' from the
        // naive lastIndexOf scan, killing completion for the next key.
        {QStringLiteral("\\coordinate (X) at (intersection cs:first line={(A)--(B)}, "), TikzCompleter::TkzCtxCoordSysKey, "next key after brace value with parens"},
        {QStringLiteral("\\coordinate (X) at (intersection cs:first line={(A)--(B)}, sec"), TikzCompleter::TkzCtxCoordSysKey, "typing next key after brace value"},
        {QStringLiteral("\\coordinate (X) at (intersection cs:first line={(0,1)--(2,3)}, "), TikzCompleter::TkzCtxCoordSysKey, "commas inside braces are not key separators"},
        // While a value is being typed the context stays silent (key side) or
        // completes coordinate names (inside the value's own parens).
        {QStringLiteral("\\coordinate (X) at (intersection cs:first line={"), TikzCompleter::TkzCtxNone, "silent right after value brace opens"},
        {QStringLiteral("\\coordinate (X) at (intersection cs:first line={(A)--(B)}, second line={(E)--(F"), TikzCompleter::TkzCtxCoord, "coordinate name inside second value"},
        // Regression: dot in closed coordinate expression (e.g. "(FF.pin 1)")
        // must NOT trigger TkzCtxDot for a later "(FF" — the dot belongs to
        // the earlier, closed group.
        {QStringLiteral("\\draw (FF.pin 1) -- ++(-1,0) node (A) {} (F"), TikzCompleter::TkzCtxCoord, "single letter in 2nd coord after closed dot ref"},
        {QStringLiteral("\\draw (FF.pin 1) -- ++(-1,0) node (A) {} (FF"), TikzCompleter::TkzCtxCoord, "FF complete in 2nd coord after closed dot ref"},
        // Still TkzCtxDot for dots in the ACTIVE coordinate ref.
        {QStringLiteral("\\draw (A) -- (B."), TikzCompleter::TkzCtxDot, "dot in open coord (no intervening ')')"},
        {QStringLiteral("\\draw (A) -- (B.in"), TikzCompleter::TkzCtxDot, "typing anchor in open coord"},
        // Regression: space after dot inside same paren must NOT trigger
        // TkzCtxDot — "(FF.pin 3 -| AND1)" has dot from "FF.pin", then " 3".
        {QStringLiteral("(FF.pin 3 -| A"), TikzCompleter::TkzCtxCoord, "space after dot → coord, not dot"},
        {QStringLiteral("(FF.pin 3 -| AN"), TikzCompleter::TkzCtxCoord, "typing 2nd name after -| with space"},
        {QStringLiteral("(FF.pin 3 -| AND1"), TikzCompleter::TkzCtxCoord, "full 2nd name after -| with space"},
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

// Verifies that both case variants of arrow tips exist in the completion
// lists — e.g. "stealth" (classic pgf) and "Stealth" (arrows.meta) are
// distinct arrow tips with different renderings and must both be offered.
static int test_arrow_case_variants_present()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    // eqCandidatesForKey(">") returns raw allArrowNames() — case variants survive.
    const QStringList arrowVals = completer.eqCandidatesForKey(QStringLiteral(">"));

    if (!arrowVals.contains(QStringLiteral("stealth"))) {
        fprintf(stderr, "FAIL: ACV-1 - '>' eq should offer 'stealth' (lowercase)\n");
        failed++;
    }
    if (!arrowVals.contains(QStringLiteral("Stealth"))) {
        fprintf(stderr, "FAIL: ACV-2 - '>' eq should offer 'Stealth' (arrows.meta)\n");
        failed++;
    }

    // Also check >= as a key.
    const QStringList geqVals = completer.eqCandidatesForKey(QStringLiteral(">="));
    if (!geqVals.contains(QStringLiteral("Stealth"))) {
        fprintf(stderr, "FAIL: ACV-3 - '>=' eq should offer 'Stealth'\n");
        failed++;
    }

    // Both must also appear in the raw arrow list.
    const QStringList allArrows = TikzWords::tikzArrows();
    if (!allArrows.contains(QStringLiteral("stealth"))) {
        fprintf(stderr, "FAIL: ACV-4 - tikzArrows() should contain 'stealth'\n");
        failed++;
    }
    if (!allArrows.contains(QStringLiteral("Stealth"))) {
        fprintf(stderr, "FAIL: ACV-5 - tikzArrows() should contain 'Stealth'\n");
        failed++;
    }

    if (failed == 0)
        fprintf(stderr, "PASS: arrow case variants present in completion\n");
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

// Regression (user report): in
//   \coordinate (X) at (intersection cs:first line={(A)--(B)}, |)
// completion after the comma offered nothing — the closed "(A)"/"(B)" pairs
// inside the finished value made the naive lastIndexOf('(') scans lose the
// governing "(intersection..." paren, so neither the context nor the system
// name ("intersection") could be determined. Keys verified against PGF 3.1.10
// tikz.code.tex (cs/first line, cs/second line, cs/first node, cs/second node,
// cs/solution).
static int test_intersection_cs_after_value()
{
    int failed = 0;

    // Helper-level checks.
    {
        const QString t = QStringLiteral(
            "\\coordinate (X) at (intersection cs:first line={(A)--(B)}, ");
        const int gp = TikzCompleter::governingOpenParenIndex(t);
        const int expected = t.indexOf(QStringLiteral("(intersection"));
        if (gp != expected) {
            fprintf(stderr, "FAIL: ICS-1 - governing paren: expected %d, got %d\n",
                    expected, gp);
            failed++;
        }
        // Commas nested in brace values do not separate keys.
        const QString seg = QStringLiteral("first line={(0,1)--(2,3)}, sec");
        const int comma = TikzCompleter::lastTopLevelCommaIndex(seg);
        if (comma != seg.lastIndexOf(QLatin1Char(','))
            || seg.mid(comma + 1).trimmed() != QStringLiteral("sec")) {
            fprintf(stderr, "FAIL: ICS-2 - top-level comma expected before 'sec'\n");
            failed++;
        }
        if (TikzCompleter::lastTopLevelCommaIndex(
                QStringLiteral("first line={(0,1)--(2,3)")) != -1) {
            fprintf(stderr, "FAIL: ICS-3 - comma inside braces must not count\n");
            failed++;
        }
    }

    // End-to-end: popup appears and offers the remaining keys.
    QPlainTextEdit editor;
    editor.show();
    TikzCompleter completer(&editor);
    TikzDocumentState state;
    completer.setDocumentState(&state);

    struct { const char *text; bool shouldPopup; const char *desc; } cases[] = {
        { "\\coordinate (X) at (intersection cs:first line={(A)--(B)}, ",
          true,  "empty segment after finished value pops keys" },
        { "\\coordinate (X) at (intersection cs:first line={(A)--(B)}, sec",
          true,  "prefix 'sec' after finished value pops keys" },
        { "\\coordinate (X) at (intersection cs:first line={(A)--(B)}, xyzq",
          false, "nonsense prefix stays silent" },
        { nullptr, false, nullptr }
    };
    for (int i = 0; cases[i].text != nullptr; ++i) {
        editor.setPlainText(QString::fromUtf8(cases[i].text));
        editor.moveCursor(QTextCursor::End);
        completer.tryComplete();
        if (completer.isPopupVisible() != cases[i].shouldPopup) {
            fprintf(stderr, "FAIL: ICS-4.%d (%s) - expected popup=%d, got %d\n",
                    i, cases[i].desc, cases[i].shouldPopup,
                    completer.isPopupVisible());
            failed++;
        }
    }

    // The key model must have been rebuilt for "intersection" — i.e. the
    // system name survived the value's nested parens.
    const QStringList keys =
        completer.modelWordsForContext(TikzCompleter::TkzCtxCoordSysKey);
    for (const char *k : { "second line", "first node", "second node", "solution" }) {
        if (!keys.contains(QString::fromUtf8(k))) {
            fprintf(stderr, "FAIL: ICS-5 - key '%s' missing from intersection cs model\n", k);
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: intersection cs key completion after finished value\n");
    return failed;
}

// CircuiTikZ bipole annotation & config keys, audited against CircuiTikZ
// 1.7.1 sources (pgfcirccurrent/voltage/flow/label, pgfcirc.defines.tex).
// Regressions covered (user report):
//   • "voltage/shift" was offered although the user-facing key is
//     "voltage shift" (\tikzset + \ctikzset, pgfcirc.defines.tex:1199-1200);
//   • only i/i_/i^ existed although current has 13 variants (i>_, i<^, ...);
//   • bogus keys "voltage/distance" / "current/shift" (nonexistent in 1.7.1).
static int test_circuitikz_annotation_keys()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    QSet<QString> ck; ck.insert(QStringLiteral("circuitikz"));
    auto optionNames = [&](const QString &env, const QString &cmd,
                           const QSet<QString> &libs) {
        QSet<QString> names;
        const auto kws = TikzKeywordDB::instance().filter(env, cmd, libs,
                                                          Category::Option);
        for (const auto *kw : kws) names.insert(kw->name);
        return names;
    };

    const QSet<QString> opts =
        optionNames(QStringLiteral("circuitikz"), QStringLiteral("draw"), ck);

    // Full annotation variant sets from the sources.
    const char *annotations[] = {
        "i","i^","i_","i<","i>","i^>","i^<","i_>","i_<","i>^","i>_","i<^","i<_",
        "v","v^","v_","v<","v>","v^>","v^<","v_>","v_<",
        "f","f^","f_","f<","f>","f^>","f^<","f_>","f_<","f>^","f>_","f<^","f<_",
        "l","l^","l_","l2","l2^","l2_","l2 above","l2 below",
        "l2 valign","l2 halign","a2 valign","a2 halign",
        "a","a^","a_","a2","a2^","a2_","a2 above","a2 below",
        "annotation","annotation above","annotation below",
        "label above","label below",
        "i symbols","no i symbols","v symbols","no v symbols",
        "f symbols","no f symbols","mirror","invert",
        nullptr
    };
    for (int i = 0; annotations[i]; ++i) {
        if (!opts.contains(QString::fromUtf8(annotations[i]))) {
            fprintf(stderr, "FAIL: CTKA-1 - annotation key '%s' missing\n",
                    annotations[i]);
            failed++;
        }
    }

    // User-facing config/style keys (\tikzset level + \ctikzset paths).
    const char *config[] = {
        "voltage shift","voltage dir","american","european",
        "american voltages","european currents","straight voltages",
        "raised voltages","cute inductors","ieee ports",
        "bipole voltage style","bipole current append style",
        "component text","bipole nodes","reversed",
        "voltage/distance from node","voltage/distance from line",
        "flow/distance","flow/offset","label/align",
        "bipoles/length","current/distance","logic ports",
        // Class-level convenience keys (pgfcircbipoles.tex + tripoles/...)
        "resistors/width","resistors/zigs","resistors/zigzag stub",
        "capacitors/width","capacitors/height",
        "inductors/width","inductors/coils",
        "sources/symbol/rotate","sources/symbol/thickness",
        "csources/symbol/rotate","csources/symbol/thickness",
        "wiper pos",
        "quadpoles/transformer/width","quadpoles/transformer/height",
        "monopoles/ground/width","multipoles/thickness",
        nullptr
    };
    for (int i = 0; config[i]; ++i) {
        if (!opts.contains(QString::fromUtf8(config[i]))) {
            fprintf(stderr, "FAIL: CTKA-2 - config key '%s' missing\n", config[i]);
            failed++;
        }
    }

    // Bogus keys must be gone.
    for (const char *bogus : { "voltage/shift", "voltage/distance", "current/shift" }) {
        if (opts.contains(QString::fromUtf8(bogus))) {
            fprintf(stderr, "FAIL: CTKA-3 - bogus key '%s' still offered\n", bogus);
            failed++;
        }
    }

    // Nonexistent voltage variants must not be invented (only current/flow
    // have the arrow-first forms).
    for (const char *bogus : { "v>^", "v>_", "v<^", "v<_" }) {
        if (opts.contains(QString::fromUtf8(bogus))) {
            fprintf(stderr, "FAIL: CTKA-4 - invented voltage variant '%s'\n", bogus);
            failed++;
        }
    }

    // voltage dir choice values (pgfcirc.defines.tex:895-899).
    {
        const auto *kw = TikzKeywordDB::instance().find(
            QStringLiteral("voltage dir"), Category::Option);
        if (!kw) {
            fprintf(stderr, "FAIL: CTKA-5 - 'voltage dir' not found\n");
            failed++;
        } else {
            for (const char *v : { "old", "noold", "RP", "EF" }) {
                if (!kw->valueHints.contains(QString::fromUtf8(v))) {
                    fprintf(stderr, "FAIL: CTKA-5 - voltage dir value '%s' missing\n", v);
                    failed++;
                }
            }
        }
    }

    // Also available in a plain tikzpicture when the package is loaded…
    {
        const QSet<QString> plain =
            optionNames(QStringLiteral("tikzpicture"), QStringLiteral("draw"), ck);
        if (!plain.contains(QStringLiteral("i>_"))
            || !plain.contains(QStringLiteral("voltage shift"))) {
            fprintf(stderr, "FAIL: CTKA-6 - keys missing in plain tikzpicture with lib\n");
            failed++;
        }
    }
    // …but never without the circuitikz lib.
    {
        const QSet<QString> none = optionNames(
            QStringLiteral("tikzpicture"), QStringLiteral("draw"), QSet<QString>());
        for (const char *k : { "i>_", "voltage shift", "f<^", "bipoles/length" }) {
            if (none.contains(QString::fromUtf8(k))) {
                fprintf(stderr, "FAIL: CTKA-7 - '%s' leaked without circuitikz\n", k);
                failed++;
            }
        }
    }

    // Per-component node/path modifier keys mirrored to /tikz/ by the sources
    // (user report: node[op amp, noinv input up] got no completion).
    const char *modifiers[] = {
        "noinv input up","noinv input down","noinv output up","noinv output down",
        "arrowmos","noarrowmos","bodydiode","nobodydiode","bulk","nobulk",
        "schottky base","no schottky base","emptycircle","fullcircle",
        "photo","filament","anodedot","fullcathode",
        "num pins","number inputs","hide numbers","show numbers",
        "external pins width","no topmark","solderdot","nogrid",
        "box","boxed","box only","boxed only",
        "t","t1","t2","text in","text out",
        nullptr
    };
    for (int i = 0; modifiers[i]; ++i) {
        if (!opts.contains(QString::fromUtf8(modifiers[i]))) {
            fprintf(stderr, "FAIL: CTKA-8 - component modifier '%s' missing\n",
                    modifiers[i]);
            failed++;
        }
    }

    // Component-class styling matrix (user report: \ctikzset{component
    // text=left, amplifiers/fill=cyan!20} offered no completion for
    // 'amplifiers/fill').
    const char *classKeys[] = {
        "amplifiers/fill","amplifiers/scale","amplifiers/thickness",
        "RF/fill","RF/thickness","power supplies/scale","grounds/fill",
        "muxdemuxes/thickness","tubes/fill","batteries/scale",
        "resistors/fill","resistors/scale","resistors/modifier thickness",
        "transistor bodydiode/relative thickness",
        "transformer core/relative thickness",
        "quadpoles/thickness","seven seg/thickness",
        nullptr
    };
    for (int i = 0; classKeys[i]; ++i) {
        if (!opts.contains(QString::fromUtf8(classKeys[i]))) {
            fprintf(stderr, "FAIL: CTKA-9 - class styling key '%s' missing\n",
                    classKeys[i]);
            failed++;
        }
    }
    // Reserved pseudo-classes are marked "do not touch" in the sources.
    for (const char *k : { "default/fill", "none/thickness" }) {
        if (opts.contains(QString::fromUtf8(k))) {
            fprintf(stderr, "FAIL: CTKA-10 - reserved key '%s' must not be offered\n", k);
            failed++;
        }
    }

    // CircuiTikZ user commands (\ctikzset & co.), lib-gated.
    {
        auto commandNames = [&](const QSet<QString> &libs) {
            QSet<QString> names;
            const auto kws = TikzKeywordDB::instance().filter(
                QStringLiteral("tikzpicture"), QString(), libs,
                Category::Command);
            for (const auto *kw : kws) names.insert(kw->name);
            return names;
        };
        const QSet<QString> withLib = commandNames(ck);
        for (const char *c : { "ctikzset", "circuitikzset", "ctikzloadstyle",
                               "ctikzsetstyle", "ctikzgetanchor",
                               "ctikzgetdirection", "ctikzflipx",
                               "ctikzflipy", "ctikzflipxy", "ctikztextnot" }) {
            if (!withLib.contains(QString::fromUtf8(c))) {
                fprintf(stderr, "FAIL: CTKA-11 - command '\\%s' missing\n", c);
                failed++;
            }
        }
        const QSet<QString> without = commandNames(QSet<QString>());
        if (without.contains(QStringLiteral("ctikzset"))) {
            fprintf(stderr, "FAIL: CTKA-12 - \\ctikzset leaked without circuitikz\n");
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: circuitikz annotation/config key completion\n");
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

// Regression: an option value that embeds a bracketed argument inside braces
// (e.g. the arrow-tip spec >={Stealth[round]}) must not break completion for
// the rest of the SAME '[...]' option list. The ']' inside the braces is nested
// and had previously fooled both the back-tracking scan and detectContext into
// treating the enclosing '[' as already closed, suppressing completion.
static int test_bracketed_brace_value_context()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    struct { const char *text; TikzCompleter::Context ctx; const char *desc; } cases[] = {
        // Cursor right after the comma that follows a >={...[...]} value.
        { "\\path edge [post,>={Stealth[round]},",
          TikzCompleter::TkzCtxBrk, "new key after arrow spec with brackets" },
        // Cursor while typing the next key.
        { "\\path edge [post,>={Stealth[round]},semi",
          TikzCompleter::TkzCtxBrk, "typing key after arrow spec with brackets" },
        // The multi-line pgfset-style form from the report, split across lines.
        { "\\begin{tikzpicture}[\n    auto,\n    pre/.style={<-,shorten <=1pt,>={Stealth[round]},semithick},\n    ",
          TikzCompleter::TkzCtxBrk, "new key after style def spanning lines" },
        // Value context (after '=') must still resolve past a bracketed-brace value.
        { "\\path edge [post,>={Stealth[round]},fill=",
          TikzCompleter::TkzCtxEq, "value context after arrow spec with brackets" },
        { nullptr, TikzCompleter::TkzCtxNone, nullptr }
    };

    for (int i = 0; cases[i].text != nullptr; ++i) {
        editor.setPlainText(QString::fromUtf8(cases[i].text));
        editor.moveCursor(QTextCursor::End);
        QString tb = completer.textBeforeForContext();
        TikzCompleter::Context got = completer.detectContext(tb);
        if (got != cases[i].ctx) {
            fprintf(stderr, "FAIL: BBV-%d (%s) - expected ctx %d, got %d (textBefore='%s')\n",
                    i, cases[i].desc, static_cast<int>(cases[i].ctx),
                    static_cast<int>(got), tb.toUtf8().constData());
            failed++;
        }
    }

    // The back-tracked context must reach the enclosing '[' (not stop short at
    // the ']' nested in braces).
    editor.setPlainText(QStringLiteral("\\path edge [post,>={Stealth[round]},"));
    editor.moveCursor(QTextCursor::End);
    QString tb = completer.textBeforeForContext();
    if (!tb.startsWith(QLatin1Char('['))) {
        fprintf(stderr, "FAIL: BBV-KEEP - back-track should start at the enclosing '[' (got '%s')\n",
                tb.toUtf8().constData());
        failed++;
    }

    fprintf(stderr, "%s: bracketed-brace value does not break option completion\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}
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
        "emptygeneric", "transformercore", "delayline",
        "nchenh", "nchdep", "pchdep", "coils", "vcoils",
        "pTy", "qqQ", "vQshape", "diacshape",
        // Internal bipole shape names (2nd arg of \pgfcirc@activate@bipole) that
        // are NOT valid to[...]/node[...] keys — verified to error under xelatex.
        // The spaced/shortcut forms (e.g. "cute inductor", "polar capacitor",
        // "push button", "solar") are the real keys and remain.
        "cuteinductor", "americaninductor", "europeaninductor", "polarcapacitor",
        "pushbutton", "toggleswitch", "solarsource", "batteryone", "batterytwo",
        "vcapacitor", "cutechoke", "thermistorntc", "thermistorptc", "ojumper",
        "ccapacitor", "oosource", "vsourceam", "cuteopenswitch",
        nullptr
    };
    for (int i = 0; bogus[i]; ++i) {
        if (set.contains(QString::fromUtf8(bogus[i]))) {
            fprintf(stderr, "FAIL: CTK-1 - invalid CircuiTikZ entry '%s' should be gone\n",
                    bogus[i]);
            failed++;
        }
    }

    // <name>shape forms are valid shapes declared via \pgfcircdeclarebipolescaled
    // (pgfcirc.defines.tex:697) — e.g. resistorshape, potentiometershape.
    // Only flag truly bogus entries where the base name is not itself
    // a known component.
    for (const QString &w : combined) {
        if (w.endsWith(QLatin1String("shape")) && !w.contains(' ')
            && w != QLatin1String("shape")
            && w != QLatin1String("pgfdeclareshape")) {
            QString base = w.left(w.length() - 5); // remove "shape"
            if (!set.contains(base)) {
                fprintf(stderr, "FAIL: CTK-2 - orphan shape-suffix entry '%s' (base '%s' unknown)\n",
                        w.toUtf8().constData(), base.toUtf8().constData());
                failed++;
            }
        }
    }

    // Genuine CircuiTikZ path components / shortcuts must be present (these are
    // exactly the ones a previous mis-"fix" would have wrongly removed/renamed).
    const char *validComponents[] = {
        "rmeterwa", "pR", "vR", "sR", "phR", "ldR", "thR", "Tr", "Ty",
        "elko", "solar", "closing switch", "opening switch",
        "cute choke", "cute inductor", "american inductor", "variable capacitor",
        "polar capacitor", "thermistor ntc", "thermistor ptc", "cspst",
        "R", "L", "C", "D", "V", "I", "pC", "pD",
        // Common base components that were previously missing (verified valid
        // to[...] keys under xelatex against CircuiTikZ 1.7.1).
        "resistor", "inductor", "potentiometer", "variable resistor",
        "variable inductor", "inductive sensor", "resistive sensor",
        "light dependent resistor", "vcc", "vee",
        // Inline port components
        "inline not", "inline buffer", "inline schmitt", "inline invschmitt",
        "inline tgate", "inline double tgate", "inline proximeter",
        // Diode-family full/empty/stroke variants (pgfcircbipoles.tex:4794-4836)
        "full diode", "empty diode", "stroke diode",
        "Do", "D-", "D*", "leDo", "leD-", "leD*", "pDo", "pD-", "pD*", "zDo", "zD-", "zD*",
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
        "schmitt port", "invschmitt port", "tgate", "double tgate",
        "ieee tgate", "ieee double tgate", "potentiometershape",
        "transformer", "transformer core", "gyrator", "coupler", "fourport",
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
            if (kw->name == QLatin1String("rmeterwa") || kw->name == QLatin1String("pR")
                || kw->name == QLatin1String("inline not")
                || kw->name == QLatin1String("inline tgate")) {
                fprintf(stderr, "FAIL: CTK-5 - '%s' leaked without circuitikz lib active\n",
                        kw->name.toUtf8().constData());
                failed++;
            }
        }
        QSet<QString> ckLibs; ckLibs.insert(QStringLiteral("circuitikz"));
        auto gated = TikzKeywordDB::instance().filter(
            QStringLiteral("circuitikz"), QStringLiteral("to"), ckLibs,
            TikzKeywords::Category::Option);
        bool foundRmeter = false, foundInlineNot = false;
        for (auto *kw : gated) {
            if (kw->name == QLatin1String("rmeterwa")) foundRmeter = true;
            if (kw->name == QLatin1String("inline not")) foundInlineNot = true;
        }
        if (!foundRmeter) {
            fprintf(stderr, "FAIL: CTK-6 - 'rmeterwa' not offered with circuitikz lib active\n");
            failed++;
        }
        if (!foundInlineNot) {
            fprintf(stderr, "FAIL: CTK-7 - 'inline not' not offered with circuitikz lib active\n");
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

    // 'invert' is circuitikz-gated: must not leak into a plain picture.
    // ('mirror' is also registered as a decoration sub-option without libs,
    // so it legitimately appears; only check 'invert'.)
    {
        QSet<QString> noLibs;
        auto plain = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QStringLiteral("draw"), noLibs,
            TikzKeywords::Category::Option);
        for (auto *kw : plain) {
            if (kw->name == QLatin1String("invert")) {
                fprintf(stderr, "FAIL: STD-3 - 'invert' leaked without circuitikz lib\n");
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

    // ── Decoration sub-option keys must be present and library-gated ──
    {
        const QStringList opts = TikzKeywordDB::instance().allOptionNames();

        const char *common[] = {
            "amplitude","segment length","angle","aspect",
            "start radius","end radius","radius",
            "path has corners","reverse path",
            "raise","mirror","pre","post","pre length","post length",
            nullptr
        };
        for (int i = 0; common[i]; ++i) {
            if (!opts.contains(QString::fromUtf8(common[i]))) {
                fprintf(stderr, "FAIL: DEC-4 - common decoration key '%s' missing\n",
                        common[i]);
                failed++;
            }
        }

        struct { const char *name; const char *lib; } libKeys[] = {
            {"shape", "decorations.shapes"},
            {"shape start width", "decorations.shapes"},
            {"foot length", "decorations.footprints"},
            {"foot of", "decorations.footprints"},
            {"text", "decorations.text"},
            {"text align", "decorations.text"},
            {"mark", "decorations.markings"},
            {"mark connection node", "decorations.markings"},
            {"reset marks", "decorations.markings"},
            {nullptr, nullptr}
        };
        for (int i = 0; libKeys[i].name; ++i) {
            const QString name = QString::fromUtf8(libKeys[i].name);
            if (!opts.contains(name)) {
                fprintf(stderr, "FAIL: DEC-5 - decoration key '%s' missing\n",
                        libKeys[i].name);
                failed++;
            }
            // Verify library gating via filter(): the key must appear when
            // its library is active and must not appear without it.
            // (find() may return the wrong entry for shared names like "mark"
            // that also exist as pgfplots options; filter() checks all.)
            {
                QSet<QString> lib; lib.insert(QString::fromUtf8(libKeys[i].lib));
                auto gated = TikzKeywordDB::instance().filter(
                    QStringLiteral("tikzpicture"), QStringLiteral("draw"), lib,
                    Category::Option);
                bool found = false;
                for (auto *kw : gated)
                    if (kw->name.compare(name, Qt::CaseInsensitive) == 0)
                    { found = true; break; }
                if (!found) {
                    fprintf(stderr, "FAIL: DEC-6 - '%s' not offered with %s lib active\n",
                            libKeys[i].name, libKeys[i].lib);
                    failed++;
                }
            }
        }
    }

    fprintf(stderr, "%s: decorations match the pgf decoration libraries\n",
            failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Verifies eqCandidatesForKey("decoration") returns all decoration names
// (not just the old hardcoded 7-name subset that was previously on the
// decoration Option key's valueHints).
static int test_decoration_eq_completion()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    const QStringList vals = completer.eqCandidatesForKey(
        QStringLiteral("decoration"));

    // Must include decoration names from multiple libraries, not just
    // a hardcoded subset.
    const char *required[] = {
        "snake","coil","zigzag","bumps","random steps","straight zigzag",
        "brace","ticks","waves","expanding waves","border",
        "show path construction","markings",
        "Koch curve type 1","Koch curve type 2","Cantor set",
        "crosses","triangles","moveto","lineto","curveto",
        nullptr
    };
    for (int i = 0; required[i]; ++i) {
        if (!vals.contains(QString::fromUtf8(required[i]))) {
            fprintf(stderr, "FAIL: DEQ-1 - 'decoration=' completion missing '%s'\n",
                    required[i]);
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: 'decoration=' eq offers all decoration names\n");
    return failed;
}

// Verifies library-specific keys (chains/spy/through/shadows/multipart/etc.)
// are registered with proper library gating.
static int test_library_keys_present()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;
    const QStringList opts = TikzKeywordDB::instance().allOptionNames();

    struct { const char *name; const char *lib; } keys[] = {
        // chains
        {"start chain", "chains"},
        {"continue chain", "chains"},
        {"on chain", "chains"},
        {"join", "chains"},
        {"chain default direction", "chains"},
        {"start branch", "chains"},
        {"continue branch", "chains"},
        // spy
        {"spy using outlines", "spy"},
        {"spy using overlays", "spy"},
        {"connect spies", "spy"},
        {"lens", "spy"},
        {"magnification", "spy"},
        {"spy connection path", "spy"},
        // through
        {"circle through", "through"},
        // shadows
        {"shadow scale", "shadows"},
        {"shadow xshift", "shadows"},
        {"shadow yshift", "shadows"},
        {"copy shadow", "shadows"},
        {"double copy shadow", "shadows"},
        {"circular glow", "shadows"},
        // shapes.multipart
        {"rectangle split draw splits", "shapes.multipart"},
        {"rectangle split part align", "shapes.multipart"},
        {"rectangle split part fill", "shapes.multipart"},
        {"rectangle split use custom fill", "shapes.multipart"},
        // shapes.symbols
        {"shape border uses incircle", "shapes.symbols"},
        // shapes.callouts
        {"callout absolute pointer", "shapes.callouts"},
        {"callout relative pointer", "shapes.callouts"},
        // shapes.arrows
        {"arrow box arrows", "shapes.arrows"},
        // trees
        {"sibling angle", "trees"},
        // fit
        {"rotate fit", "fit"},
        {"every fit", "fit"},
        // patterns.meta
        {"patterns/tile size", "patterns.meta"},
        {"patterns/bounding box", "patterns.meta"},
        {"patterns/top right", "patterns.meta"},
        {"patterns/bottom left", "patterns.meta"},
        // perspective
        {"3d view", "perspective"},
        // shapes.gates.logic
        {"use US style logic gates", "shapes.gates.logic.US"},
        {"use IEC style logic gates", "shapes.gates.logic.IEC"},
        {"and gate", "shapes.gates.logic.US"},
        {"and gate symbol", "shapes.gates.logic.IEC"},
        // mindmap
        {"root concept", "mindmap"},
        {"concept connection", "mindmap"},
        {"extra concept", "mindmap"},
        {"circle connection bar", "mindmap"},
        // animations
        {"animate", "animations"},
        {"make snapshot of", "animations"},
        // lindenmayersystems
        {"lindenmayer system", "lindenmayersystems"},
        // folding
        {"folding line length", "folding"},
        // turtle
        {"turtle/distance", "turtle"},
        {"turtle/forward", "turtle"},
        {"turtle/right", "turtle"},
        // rdf
        {"rdf engine", "rdf"},
        // views
        {"meet", "views"},
        // petri
        {"place", "petri"},
        {"every place", "petri"},
        {"transition", "petri"},
        {"every transition", "petri"},
        {"token", "petri"},
        {"every token", "petri"},
        {"pre and post", "petri"},
        {"tokens", "petri"},
        {"colored tokens", "petri"},
        {"structured tokens", "petri"},
        {"children are tokens", "petri"},
        {"token distance", "petri"},
        // automata
        {"state", "automata"},
        {"state with output", "automata"},
        {"accepting by arrow", "automata"},
        {"accepting where", "automata"},
        {"initial by diamond", "automata"},
        {"initial text", "automata"},
        // backgrounds
        {"framed", "backgrounds"},
        {"gridded", "backgrounds"},
        {"show background grid", "backgrounds"},
        {"tight background", "backgrounds"},
        {"inner frame sep", "backgrounds"},
        {"outer frame xsep", "backgrounds"},
        // positioning
        {"base left", "positioning"},
        {"mid right", "positioning"},
        // trees
        {"edge from parent fork down", "trees"},
        {"grow via three points", "trees"},
        {"clockwise from", "trees"},
        // er
        {"entity", "er"},
        {"relationship", "er"},
        {"key attribute", "er"},
        // matrix
        {"above delimiter", "matrix"},
        {"nodes in empty cells", "matrix"},
        // shadings
        {"upper left", "shadings"},
        {"lower right", "shadings"},
        // mindmap (extended)
        {"small mindmap", "mindmap"},
        {"level 2 concept", "mindmap"},
        {"circle connection bar switch color", "mindmap"},
        // 3d
        {"canvas is xz plane at y", "3d"},
        {"plane origin", "3d"},
        // quotes
        {"node quotes mean", "quotes"},
        {"quotes mean pin", "quotes"},
        // decorations.text
        {"text effects", "decorations.text"},
        {"group letters", "decorations.text"},
        {"reverse text", "decorations.text"},
        // calendar
        {"day list right", "calendar"},
        {"week list", "calendar"},
        {"month label above centered", "calendar"},
        // turtle
        {"fd", "turtle"},
        {"rt", "turtle"},
        // lindenmayersystems
        {"axiom", "lindenmayersystems"},
        {"l-system", "lindenmayersystems"},
        // intersections
        {"name path local", "intersections"},
        // perspective
        {"isometric view", "perspective"},
        // spy
        {"spy scope", "spy"},
        // views
        {"slice", "views"},
        // shadows
        {"general shadow", "shadows"},
        // shapes.gates.logic.IEC
        {"logic gate symbol color", "shapes.gates.logic.IEC"},
        // rdf
        {"has type", "rdf"},
        {"is a bag", "rdf"},
        {nullptr, nullptr}
    };

    for (int i = 0; keys[i].name; ++i) {
        const QString name = QString::fromUtf8(keys[i].name);
        if (!opts.contains(name)) {
            fprintf(stderr, "FAIL: LIB-1 - '%s' (%s) missing from options\n",
                    keys[i].name, keys[i].lib);
            failed++;
        }

        // Verify library gating via filter()
        QSet<QString> lib; lib.insert(QString::fromUtf8(keys[i].lib));
        auto gated = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QString(), lib, Category::Option);
        bool found = false;
        for (auto *kw : gated)
            if (kw->name.compare(name, Qt::CaseInsensitive) == 0)
            { found = true; break; }
        if (!found) {
            fprintf(stderr, "FAIL: LIB-2 - '%s' not offered with %s active\n",
                    keys[i].name, keys[i].lib);
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: library-specific keys registered and gated\n");
    return failed;
}

// Petri net library: place/transition/token styles and token-count options must
// be offered in node option context when \usetikzlibrary{petri} is active, and
// must NOT appear when the library is absent. Mirrors the reported example
// \node [transition,tokens=2] (n) [left=of critical] {};
static int test_petri_library_gated()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    const char *petriKeys[] = {
        "place", "transition", "token", "every place", "every transition",
        "every token", "tokens", "colored tokens", "structured tokens",
        "children are tokens", "token distance", "pre and post", nullptr
    };

    QSet<QString> withPetri; withPetri.insert(QStringLiteral("petri"));
    QSet<QString> noLibs;

    for (int i = 0; petriKeys[i]; ++i) {
        const QString name = QString::fromUtf8(petriKeys[i]);

        // Present in node option context when petri is active.
        auto gated = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QStringLiteral("node"),
            withPetri, Category::Option);
        bool found = false;
        for (auto *kw : gated)
            if (kw->name.compare(name, Qt::CaseInsensitive) == 0) { found = true; break; }
        if (!found) {
            fprintf(stderr, "FAIL: PETRI-1 - '%s' not offered in node context with petri active\n",
                    petriKeys[i]);
            failed++;
        }

        // Absent when petri is not loaded.
        auto ungated = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QStringLiteral("node"),
            noLibs, Category::Option);
        for (auto *kw : ungated) {
            if (kw->name.compare(name, Qt::CaseInsensitive) == 0) {
                fprintf(stderr, "FAIL: PETRI-2 - '%s' offered without petri library\n",
                        petriKeys[i]);
                failed++;
                break;
            }
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: petri library styles/options gated correctly\n");
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
    // 'math nodes' is NOT a tikz-cd key, but it IS a genuine graphs library
    // option (verified in tikzlibrarygraphs.code.tex). Assert it is gated to the
    // 'graphs' library rather than absent everywhere.
    {
        const auto *k = db.find(QStringLiteral("math nodes"), Category::Option);
        if (!k) {
            fprintf(stderr, "FAIL: PKG - 'math nodes' should exist as a graphs option\n");
            failed++;
        } else if (!k->requiredLibs.contains(QStringLiteral("graphs"))
                   || k->environments.contains(QStringLiteral("tikzcd"))) {
            fprintf(stderr, "FAIL: PKG - 'math nodes' must be a graphs (not tikz-cd) option\n");
            failed++;
        }
    }

    // ── tkz-euclide ──
    for (const char *c : {"tkzInit","tkzDefPoint","tkzDefLine","tkzDefCircle",
                          "tkzDefTriangle","tkzDefSquare","tkzDefRegPolygon",
                          "tkzDrawPolygon","tkzDrawCircle","tkzDrawArc","tkzDrawSegment",
                          "tkzInterLL","tkzInterLC","tkzInterCC","tkzMarkRightAngle",
                          "tkzLabelPoints","tkzDefTriangleCenter","tkzClipCircle"})
        reqCmd(c, "tkz-euclide");
    // v5.10c commands added after full source cross-check (verified defined
    // under xelatex): transformations, tangents, random points, extra
    // triangles/circles, sector variants, tests, vectors.
    for (const char *c : {"tkzTgtAt","tkzTgtFromP","tkzRandPointOnCircle",
                          "tkzRandPointOnDisk","tkzDefFeuerbachTriangle",
                          "tkzDefSimilitudeCenter","tkzDefCircleRotation",
                          "tkzDrawSectorRotate","tkzFillSectorAngles",
                          "tkzTestInterCC","tkzVecKOrth","tkzHomo","tkzSymOrth",
                          "tkzPowerCircle","tkzNagelCenter","tkzInterLLxy"})
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

// With \usetikzlibrary{intersections}, \path[name intersections={...}] auto-
// names coordinates intersection-1, intersection-2, ... These must be offered
// as coordinate-completion examples when the library is active, and must NOT
// appear otherwise.
static int test_intersections_coord_completion()
{
    int failed = 0;

    // Library active -> intersection-1/-2 offered.
    {
        QPlainTextEdit editor;
        TikzCompleter completer(&editor);
        QTextDocument doc;
        doc.setPlainText(
            "\\usetikzlibrary{intersections}\n"
            "\\path[name path=D] (0,0) -- (2,2);\n"
            "\\path[name path=E] (0,2) -- (2,0);\n"
            "\\path[name intersections={of=D and E}];\n"
            "\\coordinate (C) at (0,0);\n");
        TikzDocumentState state;
        state.reparse(&doc);
        completer.setDocumentState(&state);
        completer.updateUserModels();
        const QStringList coords =
            completer.modelWordsForContext(TikzCompleter::TkzCtxCoord);
        if (!coords.contains(QStringLiteral("intersection-1"))) {
            fprintf(stderr, "FAIL: ISEC-1 - coord model should offer 'intersection-1'\n");
            failed++;
        }
        if (!coords.contains(QStringLiteral("intersection-2"))) {
            fprintf(stderr, "FAIL: ISEC-2 - coord model should offer 'intersection-2'\n");
            failed++;
        }
        if (!coords.contains(QStringLiteral("C"))) {
            fprintf(stderr, "FAIL: ISEC-3 - real user coord 'C' still offered\n");
            failed++;
        }
    }

    // Library not loaded -> no intersection-N entries.
    {
        QPlainTextEdit editor;
        TikzCompleter completer(&editor);
        QTextDocument doc;
        doc.setPlainText("\\coordinate (C) at (0,0);\n");
        TikzDocumentState state;
        state.reparse(&doc);
        completer.setDocumentState(&state);
        completer.updateUserModels();
        const QStringList coords =
            completer.modelWordsForContext(TikzCompleter::TkzCtxCoord);
        if (coords.contains(QStringLiteral("intersection-1"))) {
            fprintf(stderr, "FAIL: ISEC-4 - 'intersection-1' offered without intersections lib\n");
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: intersections library offers intersection-N coords\n");
    return failed;
}

// Verifies TikZ coordinate-system completion: the "(name cs:)" names appear in
// the coordinate model (library-gated for 3d/calc/perspective), and the per-
// system option keys are offered after the "cs:" marker. Values verified
// against PGF/TikZ 3.1.10 sources (tikz.code.tex, tikzlibrary3d.code.tex,
// tikzlibrarycalc.code.tex, tikzlibraryperspective.code.tex).
static int test_coordinate_system_completion()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    // ── Core systems always registered ──
    const QStringList csNames = TikzKeywordDB::instance().allCoordSysNames();
    const char *core[] = { "canvas", "canvas polar", "xyz", "xyz polar",
                           "xy polar", "node", "barycentric",
                           "intersection", "perpendicular", nullptr };
    for (int i = 0; core[i]; ++i) {
        if (!csNames.contains(QString::fromUtf8(core[i]))) {
            fprintf(stderr, "FAIL: CS-1 - core coord system '%s' missing\n", core[i]);
            failed++;
        }
    }
    // Library-gated systems must be registered too (gating checked below).
    const char *gated[] = { "xyz cylindrical", "xyz spherical", "tangent",
                            "three point perspective", "tpp", nullptr };
    for (int i = 0; gated[i]; ++i) {
        if (!csNames.contains(QString::fromUtf8(gated[i]))) {
            fprintf(stderr, "FAIL: CS-2 - gated coord system '%s' missing\n", gated[i]);
            failed++;
        }
    }

    // ── 3d systems: names appear in coord model only when 3d is active ──
    {
        QPlainTextEdit editor;
        TikzCompleter completer(&editor);
        QTextDocument doc;
        doc.setPlainText("\\usetikzlibrary{3d}\n\\draw (0,0,0);\n");
        TikzDocumentState state;
        state.reparse(&doc);
        completer.setDocumentState(&state);
        completer.updateUserModels();
        const QStringList coords =
            completer.modelWordsForContext(TikzCompleter::TkzCtxCoord);
        if (!coords.contains(QStringLiteral("xyz cylindrical cs:"))) {
            fprintf(stderr, "FAIL: CS-3 - 'xyz cylindrical cs:' not offered with 3d lib\n");
            failed++;
        }
        if (!coords.contains(QStringLiteral("xyz spherical cs:"))) {
            fprintf(stderr, "FAIL: CS-4 - 'xyz spherical cs:' not offered with 3d lib\n");
            failed++;
        }
        // Core systems always present.
        if (!coords.contains(QStringLiteral("xyz polar cs:"))) {
            fprintf(stderr, "FAIL: CS-5 - core 'xyz polar cs:' not offered\n");
            failed++;
        }
    }

    // ── Without 3d library: cylindrical/spherical must NOT appear ──
    {
        QPlainTextEdit editor;
        TikzCompleter completer(&editor);
        QTextDocument doc;
        doc.setPlainText("\\draw (0,0);\n");
        TikzDocumentState state;
        state.reparse(&doc);
        completer.setDocumentState(&state);
        completer.updateUserModels();
        const QStringList coords =
            completer.modelWordsForContext(TikzCompleter::TkzCtxCoord);
        if (coords.contains(QStringLiteral("xyz cylindrical cs:"))) {
            fprintf(stderr, "FAIL: CS-6 - 'xyz cylindrical cs:' offered without 3d lib\n");
            failed++;
        }
        // But core polar system is always available.
        if (!coords.contains(QStringLiteral("canvas polar cs:"))) {
            fprintf(stderr, "FAIL: CS-7 - core 'canvas polar cs:' not offered\n");
            failed++;
        }
    }

    // ── Per-system option keys ──
    {
        QPlainTextEdit editor;
        TikzCompleter completer(&editor);
        QTextDocument doc;
        doc.setPlainText("\\usetikzlibrary{3d,calc,perspective}\n");
        TikzDocumentState state;
        state.reparse(&doc);
        completer.setDocumentState(&state);

        auto hasKeys = [&](const QString &sys, const QStringList &need,
                           const char *tag) {
            const QStringList keys = completer.coordSysKeysForName(sys);
            for (const QString &k : need) {
                if (!keys.contains(k)) {
                    fprintf(stderr, "FAIL: %s - '%s' should offer key '%s'\n",
                            tag, qPrintable(sys), qPrintable(k));
                    failed++;
                }
            }
        };
        hasKeys(QStringLiteral("xyz cylindrical"),
                {"angle","radius","z"}, "CS-8");
        hasKeys(QStringLiteral("xyz spherical"),
                {"angle","radius","latitude","longitude"}, "CS-9");
        hasKeys(QStringLiteral("canvas"), {"x","y"}, "CS-10");
        hasKeys(QStringLiteral("canvas polar"),
                {"angle","radius","x radius","y radius"}, "CS-11");
        hasKeys(QStringLiteral("tangent"), {"node","point"}, "CS-12");
        hasKeys(QStringLiteral("three point perspective"),
                {"x","y","z"}, "CS-13");

        // 'z' is NOT a key of "xyz spherical" (uses latitude/longitude).
        if (completer.coordSysKeysForName(QStringLiteral("xyz spherical"))
                .contains(QStringLiteral("z"))) {
            fprintf(stderr, "FAIL: CS-14 - 'xyz spherical' must not offer 'z'\n");
            failed++;
        }
    }

    // ── Key model built from the system name preceding 'cs:' ──
    {
        QPlainTextEdit editor;
        TikzCompleter completer(&editor);
        QTextDocument doc;
        doc.setPlainText("\\usetikzlibrary{3d}\n");
        TikzDocumentState state;
        state.reparse(&doc);
        completer.setDocumentState(&state);
        completer.updateCoordSysKeyModel(QStringLiteral("xyz cylindrical"));
        const QStringList keys =
            completer.modelWordsForContext(TikzCompleter::TkzCtxCoordSysKey);
        if (!keys.contains(QStringLiteral("angle"))
            || !keys.contains(QStringLiteral("z"))) {
            fprintf(stderr, "FAIL: CS-15 - key model missing cylindrical keys\n");
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: coordinate-system name + key completion (3d/calc/perspective)\n");
    return failed;
}

// Verifies anchors are source-accurate against TeXLive PGF/circuitikz:
// universal PGF anchors always present, circuitikz anchors gated, and
// bogus/hallucinated entries removed (angle numbers, "substrate",
// "primary/secondary", "text split one", "part one", single-letter
// non-anchors "A"/"Y"/"O", etc.).
static int test_anchors_source_accurate()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    const QStringList anchors = TikzKeywordDB::instance().allAnchorNames();

    // ── Universal PGF anchors (always present) ──
    const char *universal[] = {
        "north","north east","north west","south","south east","south west",
        "east","west","center","text",
        "base","base east","base west",
        "mid","mid east","mid west",
        nullptr
    };
    for (int i = 0; universal[i]; ++i) {
        if (!anchors.contains(QString::fromLatin1(universal[i]))) {
            fprintf(stderr, "FAIL: ANC-1 - universal anchor '%s' missing\n",
                    universal[i]);
            failed++;
        }
    }

    // ── Circuitikz anchors (present, gated by circuitikz lib) ──
    const char *ckt[] = {
        "wiper","W","cathode","anode","gate","G",
        "in","in 1","in 2","out","out 1","out 2",
        "tap","tap down","tap up","v+","v-","tip",
        "B","C","E","S","D",
        "collector","emitter","source","drain","bulk",
        "+","-","left","right","top","bottom",
        // Quadpole transformer port anchors (pgfcircquadpoles.tex:134-146)
        "A1","A2","B1","B2",
        "AA1","AA2","BB1","BB2",
        "inner dot A1","inner dot A2","inner dot B1","inner dot B2",
        "outer dot A1","outer dot A2","outer dot B1","outer dot B2",
        nullptr
    };
    for (int i = 0; ckt[i]; ++i) {
        QString name = QString::fromLatin1(ckt[i]);
        if (!anchors.contains(name)) {
            fprintf(stderr, "FAIL: ANC-2 - circuitikz anchor '%s' missing\n",
                    ckt[i]);
            failed++;
        }
        // Verify library gating
        const auto *kw = TikzKeywordDB::instance().find(name, Category::Anchor);
        if (!kw || !kw->requiredLibs.contains(QStringLiteral("circuitikz"))) {
            fprintf(stderr, "FAIL: ANC-3 - '%s' must require circuitikz lib\n",
                    ckt[i]);
            failed++;
        }
    }

    // ── Bogus entries (must be REMOVED) ──
    const char *bogus[] = {
        "substrate",
        "primary","secondary","primary left","primary right",
        "secondary left","secondary right",
        "text split","text split one","text split two",
        "part one","part two","part three","part four",
        "corner 1","corner 2",
        "input 1","input 2","output 1","output 2",
        "angle","upper",
        nullptr
    };
    for (int i = 0; bogus[i]; ++i) {
        if (anchors.contains(QString::fromLatin1(bogus[i]))) {
            fprintf(stderr, "FAIL: ANC-4 - bogus anchor '%s' should be removed\n",
                    bogus[i]);
            failed++;
        }
    }

    // Single-letter non-anchors from the old list that were removed.
    // (B, C, E, S, D, G, W are kept — they are real circuitikz anchors.)
    const char *bogusLetters[] = { "A", "Y", "O", nullptr };
    for (int i = 0; bogusLetters[i]; ++i) {
        if (anchors.contains(QString::fromLatin1(bogusLetters[i]))) {
            fprintf(stderr, "FAIL: ANC-5 - single-letter bogus anchor '%s' should be removed\n",
                    bogusLetters[i]);
            failed++;
        }
    }

    // Angle numbers: removed because ANY angle is valid, not just the 11 listed.
    const char *angles[] = { "0","30","60","90","120","150",
                              "210","240","270","300","330", nullptr };
    for (int i = 0; angles[i]; ++i) {
        if (anchors.contains(QString::fromLatin1(angles[i]))) {
            fprintf(stderr, "FAIL: ANC-6 - angle number '%s' should not be a named anchor\n",
                    angles[i]);
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: anchors source-accurate against TeXLive PGF/circuitikz\n");
    return failed;
}

// Verifies filter() for anchors respects library gating: circuitikz anchors
// are hidden when circuitikz lib is inactive, and shown when active.
static int test_anchor_library_gating()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    // Without circuitikz lib: must NOT see circuitikz anchors
    {
        QSet<QString> noLibs;
        auto plain = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QStringLiteral("node"), noLibs,
            Category::Anchor);
        QSet<QString> names;
        for (auto *kw : plain) names.insert(kw->name);

        if (names.contains(QStringLiteral("wiper"))) {
            fprintf(stderr, "FAIL: ALG-1 - 'wiper' leaked without circuitikz lib\n");
            failed++;
        }
        if (names.contains(QStringLiteral("cathode"))) {
            fprintf(stderr, "FAIL: ALG-2 - 'cathode' leaked without circuitikz lib\n");
            failed++;
        }
        if (names.contains(QStringLiteral("B"))) {
            fprintf(stderr, "FAIL: ALG-3 - 'B' leaked without circuitikz lib\n");
            failed++;
        }

        // Universal anchors must still be present
        if (!names.contains(QStringLiteral("north"))) {
            fprintf(stderr, "FAIL: ALG-4 - 'north' missing without circuitikz\n");
            failed++;
        }
        if (!names.contains(QStringLiteral("center"))) {
            fprintf(stderr, "FAIL: ALG-5 - 'center' missing without circuitikz\n");
            failed++;
        }
    }

    // With circuitikz lib active: must see circuitikz anchors
    {
        QSet<QString> ckLibs; ckLibs.insert(QStringLiteral("circuitikz"));
        auto gated = TikzKeywordDB::instance().filter(
            QStringLiteral("circuitikz"), QStringLiteral("node"), ckLibs,
            Category::Anchor);
        QSet<QString> names;
        for (auto *kw : gated) names.insert(kw->name);

        if (!names.contains(QStringLiteral("wiper"))) {
            fprintf(stderr, "FAIL: ALG-6 - 'wiper' not offered with circuitikz lib\n");
            failed++;
        }
        if (!names.contains(QStringLiteral("cathode"))) {
            fprintf(stderr, "FAIL: ALG-7 - 'cathode' not offered with circuitikz lib\n");
            failed++;
        }
        if (!names.contains(QStringLiteral("B"))) {
            fprintf(stderr, "FAIL: ALG-8 - 'B' not offered with circuitikz lib\n");
            failed++;
        }
        // Universal still present
        if (!names.contains(QStringLiteral("north"))) {
            fprintf(stderr, "FAIL: ALG-9 - 'north' missing with circuitikz\n");
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: anchor library gating (circuitikz on/off)\n");
    return failed;
}

// Verify circuitikz's dynamically-created pin anchors (pin 1–3, bpin 1–3)
// for flipflop/dipchip/muxdemux shapes (pgfcircmultipoles.tex).
static int test_circuitikz_pin_anchors()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);
    TikzDocumentState state;
    QTextDocument doc;
    doc.setPlainText(
        "\\begin{circuitikz}\n"
        "\\draw (0,0) node[flipflop] (FF) {};\n"
        "\\end{circuitikz}\n");
    state.reparse(&doc);
    completer.setDocumentState(&state);
    completer.updateUserModels();

    const QStringList anchors = completer.modelWordsForContext(TikzCompleter::TkzCtxDot);
    if (!anchors.contains(QStringLiteral("pin 1"))) {
        fprintf(stderr, "FAIL: CPA-1 - 'pin 1' missing from dot model with circuitikz\n");
        failed++;
    }
    if (!anchors.contains(QStringLiteral("pin 2"))) {
        fprintf(stderr, "FAIL: CPA-2 - 'pin 2' missing from dot model\n");
        failed++;
    }
    if (!anchors.contains(QStringLiteral("pin 3"))) {
        fprintf(stderr, "FAIL: CPA-3 - 'pin 3' missing from dot model\n");
        failed++;
    }
    if (!anchors.contains(QStringLiteral("bpin 1"))) {
        fprintf(stderr, "FAIL: CPA-4 - 'bpin 1' missing from dot model\n");
        failed++;
    }

    // Verify not leaking without circuitikz.
    TikzDocumentState state2;
    QTextDocument doc2;
    doc2.setPlainText("\\begin{tikzpicture}\n\\end{tikzpicture}\n");
    state2.reparse(&doc2);
    completer.setDocumentState(&state2);
    completer.updateUserModels();
    const QStringList anchorsNoCtk = completer.modelWordsForContext(TikzCompleter::TkzCtxDot);
    if (anchorsNoCtk.contains(QStringLiteral("pin 1"))) {
        fprintf(stderr, "FAIL: CPA-5 - 'pin 1' leaked without circuitikz\n");
        failed++;
    }

    if (failed == 0)
        fprintf(stderr, "PASS: circuitikz pin anchors (pin 1–3, bpin 1–3)\n");
    return failed;
}

// Feature 2: the 'label=' key must offer positional value completions (above,
// above left, left, ...) so \node[label=|] can complete a compass/direction.
// 'pin=' shares the same positional hints.
static int test_label_position_completion()
{
    int failed = 0;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    const QStringList vals = completer.eqCandidatesForKey(QStringLiteral("label"));
    const char *positions[] = {
        "above", "below", "left", "right",
        "above left", "above right", "below left", "below right",
        "center", nullptr
    };
    for (int i = 0; positions[i]; ++i) {
        if (!vals.contains(QString::fromUtf8(positions[i]))) {
            fprintf(stderr, "FAIL: LBL-1 - 'label=' should offer position '%s'\n",
                    positions[i]);
            failed++;
        }
    }
    // 'pin=' shares the positional hints.
    const QStringList pinVals = completer.eqCandidatesForKey(QStringLiteral("pin"));
    if (!pinVals.contains(QStringLiteral("above left"))) {
        fprintf(stderr, "FAIL: LBL-2 - 'pin=' should offer 'above left'\n");
        failed++;
    }
    if (failed == 0)
        fprintf(stderr, "PASS: label/pin position value completion (Feature 2)\n");
    return failed;
}

// Feature 4: 'name path' (and global/local variants) must be offered in \node
// option context with the intersections library active, not only on \draw/\path.
static int test_name_path_in_node()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    QSet<QString> libs; libs.insert(QStringLiteral("intersections"));

    auto offeredIn = [&](const char *cmd, const char *key) -> bool {
        auto kws = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QString::fromUtf8(cmd), libs,
            Category::Option);
        for (auto *kw : kws)
            if (kw->name == QString::fromUtf8(key)) return true;
        return false;
    };

    for (const char *key : { "name path", "name path global", "name path local" }) {
        if (!offeredIn("node", key)) {
            fprintf(stderr, "FAIL: NPN-1 - '%s' should be offered in \\node context\n", key);
            failed++;
        }
        if (!offeredIn("draw", key)) {
            fprintf(stderr, "FAIL: NPN-2 - '%s' should still be offered in \\draw context\n", key);
            failed++;
        }
    }

    // 'name intersections' stays path-only (not a node option).
    {
        auto kws = TikzKeywordDB::instance().filter(
            QStringLiteral("tikzpicture"), QStringLiteral("node"), libs,
            Category::Option);
        for (auto *kw : kws) {
            if (kw->name == QLatin1String("name intersections")) {
                fprintf(stderr, "FAIL: NPN-3 - 'name intersections' should not be a \\node option\n");
                failed++;
            }
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: name path offered in \\node context (Feature 4)\n");
    return failed;
}

// Feature 5: the 'of=' key of name intersections must offer the user's named
// paths, and the value context must resolve inside the brace group (including
// multi-word "A and B" content).
static int test_of_path_completion()
{
    int failed = 0;
    QPlainTextEdit editor;
    editor.show();
    TikzCompleter completer(&editor);
    QTextDocument doc;
    doc.setPlainText(
        "\\usetikzlibrary{intersections}\n"
        "\\draw [name path=D--F] (0,0) -- (2,2);\n"
        "\\node [name path=circle K] (H) at (1,1) {};\n"
        "\\path [name intersections={of=}];\n");
    TikzDocumentState state;
    state.reparse(&doc);
    completer.setDocumentState(&state);

    const QStringList vals = completer.eqCandidatesForKey(QStringLiteral("of"));
    if (!vals.contains(QStringLiteral("D--F"))) {
        fprintf(stderr, "FAIL: OFP-1 - 'of=' should offer named path 'D--F'\n");
        failed++;
    }
    if (!vals.contains(QStringLiteral("circle K"))) {
        fprintf(stderr, "FAIL: OFP-2 - 'of=' should offer named path 'circle K'\n");
        failed++;
    }

    // Context: value context inside a brace group after 'of=' must resolve to
    // TkzCtxEq (so the path names are offered), even for multi-word content
    // containing " and ".
    struct { const char *text; TikzCompleter::Context ctx; const char *desc; } cases[] = {
        { "\\path [name intersections={of=",             TikzCompleter::TkzCtxEq, "of= empty" },
        { "\\path [name intersections={of=D--F and ",    TikzCompleter::TkzCtxEq, "of= after 'and'" },
        { "\\path [name intersections={of=D--F and cir", TikzCompleter::TkzCtxEq, "of= typing 2nd path" },
        { nullptr, TikzCompleter::TkzCtxNone, nullptr }
    };
    for (int i = 0; cases[i].text; ++i) {
        editor.setPlainText(QString::fromUtf8(cases[i].text));
        editor.moveCursor(QTextCursor::End);
        QString tb = completer.textBeforeForContext();
        TikzCompleter::Context got = completer.detectContext(tb);
        if (got != cases[i].ctx) {
            fprintf(stderr, "FAIL: OFP-3 (%s) - expected ctx %d, got %d (tb='%s')\n",
                    cases[i].desc, static_cast<int>(cases[i].ctx),
                    static_cast<int>(got), tb.toUtf8().constData());
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: of= path-name completion in name intersections (Feature 5)\n");
    return failed;
}

// 'font' and 'node font' keys must be registered node options and offer LaTeX
// font-command value completions (\itshape, \bfseries, \tiny, ...).
static int test_font_completion()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);

    const QStringList opts = TikzKeywordDB::instance().allOptionNames();
    if (!opts.contains(QStringLiteral("font"))) {
        fprintf(stderr, "FAIL: FNT-1 - 'font' option missing\n"); failed++;
    }
    if (!opts.contains(QStringLiteral("node font"))) {
        fprintf(stderr, "FAIL: FNT-2 - 'node font' option missing\n"); failed++;
    }

    const char *cmds[] = {
        "\\rmfamily","\\sffamily","\\ttfamily","\\bfseries","\\mdseries",
        "\\upshape","\\itshape","\\slshape","\\scshape",
        "\\tiny","\\scriptsize","\\footnotesize","\\small",
        "\\normalsize","\\large","\\Large","\\LARGE","\\huge","\\Huge", nullptr
    };
    for (const char *key : { "font", "node font" }) {
        const QStringList vals = completer.eqCandidatesForKey(QString::fromUtf8(key));
        for (int i = 0; cmds[i]; ++i) {
            if (!vals.contains(QString::fromUtf8(cmds[i]))) {
                fprintf(stderr, "FAIL: FNT-3 - '%s=' should offer '%s'\n", key, cmds[i]);
                failed++;
            }
        }
    }

    // 'node font' is valid in node context on \draw (draw shares node options).
    QSet<QString> noLibs;
    auto drawOpts = TikzKeywordDB::instance().filter(
        QStringLiteral("tikzpicture"), QStringLiteral("draw"), noLibs, Category::Option);
    bool foundNodeFont = false;
    for (auto *kw : drawOpts)
        if (kw->name == QLatin1String("node font")) { foundNodeFont = true; break; }
    if (!foundNodeFont) {
        fprintf(stderr, "FAIL: FNT-4 - 'node font' should be offered in \\draw option context\n");
        failed++;
    }

    // font/node font values must resolve to value context (TkzCtxEq) not just in
    // a bracketed option list, but also inside a style body — including when the
    // value already begins with a backslash macro (font=\it), where the generic
    // word logic would otherwise reroute to command completion. This is the
    // scenario "foo/.style={draw,font=\itshape}". textBeforeForContext truncates
    // at the enclosing '{', so we feed those brace-prefixed strings directly.
    {
        struct { const char *text; const char *desc; } eqCases[] = {
            { "{draw,font=",                        "font= in style body" },
            { "{draw,font=\\it",                    "font=\\it in style body" },
            { "{draw,font=\\bfseries\\it",          "font= run of macros" },
            { "{rounded corners=3mm,node font=",    "node font= in style body" },
            { "{rounded corners=3mm,node font=\\tt","node font=\\tt in style body" },
            { nullptr, nullptr }
        };
        for (int i = 0; eqCases[i].text; ++i) {
            TikzCompleter::Context got =
                completer.detectContext(QString::fromUtf8(eqCases[i].text));
            if (got != TikzCompleter::TkzCtxEq) {
                fprintf(stderr, "FAIL: FNT-5 (%s) - expected TkzCtxEq, got %d\n",
                        eqCases[i].desc, static_cast<int>(got));
                failed++;
            }
        }
        // Sanity: node text containing '=' must NOT be treated as a value context.
        if (completer.detectContext(QStringLiteral("{x = y"))
            == TikzCompleter::TkzCtxEq) {
            fprintf(stderr, "FAIL: FNT-6 - node text '{x = y' should not be a value context\n");
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: font / node font value completion\n");
    return failed;
}

// Regression: accepting a completion for a value that contains a space must not
// leave a stray leading character. Reproduces the reported bug where, after
// typing "right=of " (trailing space) or "below " and accepting from the popup,
// the result gained a duplicated first letter ("right=oof digit", "bbelow left").
// Root cause: the popup prefix was trimmed (dropping the trailing space) so the
// acceptance deletion undercounted the on-screen token by one+ characters.
static int test_spaced_completion_accept_no_stray_char()
{
    int failed = 0;
    QPlainTextEdit editor;
    editor.show();
    TikzCompleter completer(&editor);
    QTextDocument doc;
    doc.setPlainText(
        "\\usetikzlibrary{positioning}\n"
        "\\node (digit) at (0,0) {d};\n");
    TikzDocumentState state;
    state.reparse(&doc);
    completer.setDocumentState(&state);

    // Case 1: positioning value after "right=of " (trailing space). Accepting a
    // suggestion must not duplicate the 'o' ("=oof...").
    {
        editor.setPlainText(QStringLiteral("\\node [right=of "));
        editor.moveCursor(QTextCursor::End);
        completer.tryComplete();
        if (completer.isPopupVisible()) {
            QKeyEvent ret(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            completer.handleCompletionKey(&ret);
            const QString result = editor.toPlainText();
            const int eq = result.indexOf(QLatin1Char('='));
            const QString after = (eq >= 0) ? result.mid(eq + 1) : result;
            if (after.startsWith(QStringLiteral("oo"))
                || result.contains(QStringLiteral("=oof"))) {
                fprintf(stderr, "FAIL: SPC-1 - stray char after '=': '%s'\n",
                        result.toUtf8().constData());
                failed++;
            }
        }
    }

    // Case 2: bracket positional value after "below " (trailing space). Accepting
    // must not duplicate the 'b' ("bbelow left").
    {
        editor.setPlainText(QStringLiteral("\\node [below "));
        editor.moveCursor(QTextCursor::End);
        completer.tryComplete();
        if (completer.isPopupVisible()) {
            QKeyEvent ret(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            completer.handleCompletionKey(&ret);
            const QString result = editor.toPlainText();
            const int br = result.indexOf(QLatin1Char('['));
            const QString token = (br >= 0) ? result.mid(br + 1) : result;
            if (token.startsWith(QStringLiteral("bb"))) {
                fprintf(stderr, "FAIL: SPC-2 - duplicated leading char: '%s'\n",
                        token.toUtf8().constData());
                failed++;
            }
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: spaced completion accept leaves no stray character\n");
    return failed;
}

// The 'to path' option and the to-path coordinate macros (\tikztostart,
// \tikztotarget, \tikztonodes) must be completable. 'to path' is a general
// option (settable in \tikzset/styles, not only on \draw/\path), and the three
// macros are \backslash commands available for completion.
static int test_to_path_completion()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    // 'to path' present among option names.
    const QStringList opts = TikzKeywordDB::instance().allOptionNames();
    if (!opts.contains(QStringLiteral("to path"))) {
        fprintf(stderr, "FAIL: TP-1 - 'to path' option missing\n"); failed++;
    }

    // 'to path' must be offered even without a command context (e.g. in a
    // \tikzset/style body): filter with empty command and no libs.
    QSet<QString> noLibs;
    auto styleOpts = TikzKeywordDB::instance().filter(
        QStringLiteral("tikzpicture"), QString(), noLibs, Category::Option);
    bool foundToPath = false;
    for (auto *kw : styleOpts)
        if (kw->name == QLatin1String("to path")) { foundToPath = true; break; }
    if (!foundToPath) {
        fprintf(stderr, "FAIL: TP-2 - 'to path' should be offered in style/no-command context\n");
        failed++;
    }

    // The to-path coordinate macros must be registered \commands.
    const QStringList cmds = TikzKeywordDB::instance().allCommandNames();
    for (const char *m : { "tikztostart", "tikztotarget", "tikztonodes" }) {
        if (!cmds.contains(QString::fromUtf8(m))) {
            fprintf(stderr, "FAIL: TP-3 - to-path macro '\\%s' missing from commands\n", m);
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: to path option + \\tikztostart/\\tikztotarget/\\tikztonodes\n");
    return failed;
}

// The graphs library and \matrix option completion: grow/branch growth keys,
// placement strategies, node/edge appearance keys and matrix column/row sep.
static int test_graphs_and_matrix_options()
{
    int failed = 0;
    using TikzKeywords::TikzKeywordDB;
    using TikzKeywords::Category;

    const QStringList opts = TikzKeywordDB::instance().allOptionNames();

    // graphs library keys must be present.
    const char *graphKeys[] = {
        "grow right", "grow left", "grow up", "grow down",
        "grow right sep", "grow down sep", "branch down", "branch right sep",
        "edges", "edge", "math nodes", "empty nodes",
        "Cartesian placement", "circular placement", "no placement",
        "clockwise", "counterclockwise", "phase",
        "chain shift", "group shift", "number nodes",
        "complete bipartite", "matching", "butterfly'", "every graph",
        "default edge kind", "left anchor", "right anchor",
        // Newly added, verified against tikzlibrarygraphs(.standard).code.tex:
        "use existing nodes", "fresh nodes", "multi", "simple", "quick",
        "no edges", "trie", "clear >", "clear <",
        "put node text on incoming edges", "put node text on outgoing edges",
        "V", "W", "n", "m", "name shore V", "name shore W", "declare",
        "new ->", "new --", "new <-", "new <->", "new -!-",
        "default edge operator", "name separator", "as", "typeset",
        "chain count", "element count", "compute position", "place",
        "subgraph I_n", "subgraph I_nm", "subgraph K_n", "subgraph K_nm",
        "subgraph C_n", "subgraph P_n", "subgraph Grid_n", "subgraph G_np",
        nullptr
    };
    for (int i = 0; graphKeys[i]; ++i) {
        if (!opts.contains(QString::fromUtf8(graphKeys[i]))) {
            fprintf(stderr, "FAIL: GRA-1 - graphs key '%s' missing\n", graphKeys[i]);
            failed++;
        }
    }

    // 'group count' is NOT a real key (only chain count / element count exist in
    // the source). Make sure it did not creep back in.
    if (opts.contains(QStringLiteral("group count"))) {
        fprintf(stderr, "FAIL: GRA-1b - spurious 'group count' key present\n");
        failed++;
    }

    // graphs keys must be gated by the 'graphs' library.
    QSet<QString> graphsLib; graphsLib.insert(QStringLiteral("graphs"));
    QSet<QString> noLibs;
    auto gated = TikzKeywordDB::instance().filter(
        QStringLiteral("tikzpicture"), QString(), graphsLib, Category::Option);
    bool foundGrow = false;
    for (auto *kw : gated)
        if (kw->name == QLatin1String("grow right")) { foundGrow = true; break; }
    if (!foundGrow) {
        fprintf(stderr, "FAIL: GRA-2 - 'grow right' not offered with graphs lib\n");
        failed++;
    }
    auto ungated = TikzKeywordDB::instance().filter(
        QStringLiteral("tikzpicture"), QString(), noLibs, Category::Option);
    for (auto *kw : ungated) {
        if (kw->name == QLatin1String("grow right")) {
            fprintf(stderr, "FAIL: GRA-3 - 'grow right' leaked without graphs lib\n");
            failed++;
            break;
        }
    }

    // Newly-added graphs keys must also be gated by the 'graphs' library and
    // must not leak when no library is active.
    const char *gatedKeys[] = {
        "use existing nodes", "fresh nodes", "multi", "quick", "no edges",
        "trie", "put node text on incoming edges", "V", "W", "declare",
        "new ->", "clear >", nullptr
    };
    for (int i = 0; gatedKeys[i]; ++i) {
        const QString key = QString::fromUtf8(gatedKeys[i]);
        bool inGraphs = false, leaked = false;
        for (auto *kw : gated)
            if (kw->name == key) { inGraphs = true; break; }
        for (auto *kw : ungated)
            if (kw->name == key) { leaked = true; break; }
        if (!inGraphs) {
            fprintf(stderr, "FAIL: GRA-2b - '%s' not offered with graphs lib\n",
                    gatedKeys[i]);
            failed++;
        }
        if (leaked) {
            fprintf(stderr, "FAIL: GRA-3b - '%s' leaked without graphs lib\n",
                    gatedKeys[i]);
            failed++;
        }
    }

    // graphs.standard subgraph components must be gated by 'graphs.standard'.
    QSet<QString> stdLib; stdLib.insert(QStringLiteral("graphs.standard"));
    auto stdGated = TikzKeywordDB::instance().filter(
        QStringLiteral("tikzpicture"), QString(), stdLib,
        Category::Option);
    bool foundSubgraph = false;
    for (auto *kw : stdGated)
        if (kw->name == QLatin1String("subgraph K_nm")) { foundSubgraph = true; break; }
    if (!foundSubgraph) {
        fprintf(stderr, "FAIL: GRA-2c - 'subgraph K_nm' not offered with graphs.standard\n");
        failed++;
    }
    for (auto *kw : ungated) {
        if (kw->name == QLatin1String("subgraph K_nm")) {
            fprintf(stderr, "FAIL: GRA-3c - 'subgraph K_nm' leaked without graphs.standard\n");
            failed++;
            break;
        }
    }

    // grow/branch keys offer a distance hint.
    QPlainTextEdit editor;
    TikzCompleter completer(&editor);
    const QStringList grVals = completer.eqCandidatesForKey(QStringLiteral("grow right"));
    if (!grVals.contains(QStringLiteral("1cm"))) {
        fprintf(stderr, "FAIL: GRA-4 - 'grow right=' should offer a distance like '1cm'\n");
        failed++;
    }
    // 'default edge kind' offers the edge-kind shorthands as values.
    const QStringList dekVals = completer.eqCandidatesForKey(QStringLiteral("default edge kind"));
    if (!dekVals.contains(QStringLiteral("->"))) {
        fprintf(stderr, "FAIL: GRA-4b - 'default edge kind=' should offer '->'\n");
        failed++;
    }

    // matrix column sep / row sep present, gated by matrix, and offer hints.
    for (const char *k : { "column sep", "row sep" }) {
        if (!opts.contains(QString::fromUtf8(k))) {
            fprintf(stderr, "FAIL: GRA-5 - matrix key '%s' missing\n", k);
            failed++;
        }
    }
    QSet<QString> matrixLib; matrixLib.insert(QStringLiteral("matrix"));
    auto mGated = TikzKeywordDB::instance().filter(
        QStringLiteral("tikzpicture"), QString(), matrixLib, Category::Option);
    bool foundColSep = false;
    for (auto *kw : mGated)
        if (kw->name == QLatin1String("column sep")) { foundColSep = true; break; }
    if (!foundColSep) {
        fprintf(stderr, "FAIL: GRA-6 - 'column sep' not offered with matrix lib\n");
        failed++;
    }
    const QStringList csVals = completer.eqCandidatesForKey(QStringLiteral("column sep"));
    if (!csVals.contains(QStringLiteral("between origins"))) {
        fprintf(stderr, "FAIL: GRA-7 - 'column sep=' should offer 'between origins'\n");
        failed++;
    }
    // The value hints for a name registered by several libraries (matrix +
    // tikz-cd) must be the union: matrix distances AND tikz-cd size words.
    if (!csVals.contains(QStringLiteral("small"))
        || !csVals.contains(QStringLiteral("tiny"))) {
        fprintf(stderr, "FAIL: GRA-8 - 'column sep=' should merge tikz-cd size words (tiny/small)\n");
        failed++;
    }

    if (failed == 0)
        fprintf(stderr, "PASS: graphs library + \\matrix column/row sep completion\n");
    return failed;
}

// Regression (user report): node names declared by the PATH OPERATION form
// 'node [opts] (name)' (no backslash) were never offered in coordinate
// completion. Exact circuitikz op-amp example from the report.
static int test_path_op_node_name_completion()
{
    int failed = 0;
    QPlainTextEdit editor;
    editor.show();
    TikzCompleter completer(&editor);

    QTextDocument doc;
    doc.setPlainText(
        "\\begin{circuitikz}[scale=1.2,european]\n"
        "\\draw (0,0) node[above] {$v_i$} to [short,o-] ++ (1,0) node [op amp,noinv input up,anchor=+] (OA) {\\texttt{OA1}} ;\n"
        "\\draw (OA.-) to[short,-*] ++(0,-1) coordinate (FB) -- ++(0,-0.5)  to[R=$R_1$] ++(0,-1)  node[ground]{};\n"
        "\\draw (FB) to [R=$R_2$] (FB -| OA.out) to[short,-*] (OA.out);\n"
        "\\end{circuitikz}\n");
    TikzDocumentState state;
    state.reparse(&doc);
    completer.setDocumentState(&state);
    completer.updateUserModels();

    // The coordinate-completion model must offer both the path-op node (OA)
    // and the inline coordinate (FB).
    const QStringList coords = completer.modelWordsForContext(TikzCompleter::TkzCtxCoord);
    if (!coords.contains(QStringLiteral("OA"))) {
        fprintf(stderr, "FAIL: OPN-1 - coord model must contain path-op node 'OA'\n");
        failed++;
    }
    if (!coords.contains(QStringLiteral("FB"))) {
        fprintf(stderr, "FAIL: OPN-2 - coord model must contain 'FB'\n");
        failed++;
    }

    // Typing "(O" inside the picture must pop up and complete to OA.
    {
        editor.setPlainText(QStringLiteral("\\draw (O"));
        editor.moveCursor(QTextCursor::End);
        completer.tryComplete();
        if (!completer.isPopupVisible()) {
            fprintf(stderr, "FAIL: OPN-3 - no popup for '(O' with node OA defined\n");
            failed++;
        } else {
            QKeyEvent ret(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            completer.handleCompletionKey(&ret);
            if (!editor.toPlainText().contains(QStringLiteral("(OA"))) {
                fprintf(stderr, "FAIL: OPN-4 - accepting completion should insert OA (got '%s')\n",
                        editor.toPlainText().toUtf8().constData());
                failed++;
            }
        }
    }

    // After the perpendicular operator -| the second name must complete too:
    // "(FB -| O" → prefix "O" → OA (previously the whole "FB -| O" was used
    // as prefix and nothing matched).
    {
        editor.setPlainText(QStringLiteral("\\draw (FB) to [R=$R_2$] (FB -| O"));
        editor.moveCursor(QTextCursor::End);
        completer.tryComplete();
        if (!completer.isPopupVisible()) {
            fprintf(stderr, "FAIL: OPN-5 - no popup after '-|' operator\n");
            failed++;
        } else {
            QKeyEvent ret(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            completer.handleCompletionKey(&ret);
            if (!editor.toPlainText().contains(QStringLiteral("(FB -| OA"))) {
                fprintf(stderr, "FAIL: OPN-6 - '-|' completion mangled text: '%s'\n",
                        editor.toPlainText().toUtf8().constData());
                failed++;
            }
        }
    }

    // "(FB -| OA." must switch to anchor (dot) completion — circuitikz is
    // active via \begin{circuitikz}, so op-amp anchors like 'out' are offered.
    {
        const TikzCompleter::Context ctx = completer.detectContext(
            QStringLiteral("\\draw (FB) to [R=$R_2$] (FB -| OA."));
        if (ctx != TikzCompleter::TkzCtxDot) {
            fprintf(stderr, "FAIL: OPN-7 - '(FB -| OA.' should be Dot context (got %d)\n",
                    static_cast<int>(ctx));
            failed++;
        }
        completer.updateUserModels();
        const QStringList anchors = completer.modelWordsForContext(TikzCompleter::TkzCtxDot);
        if (!anchors.contains(QStringLiteral("out"))) {
            fprintf(stderr, "FAIL: OPN-8 - 'out' anchor missing (circuitikz active via env)\n");
            failed++;
        }
        if (!anchors.contains(QStringLiteral("+")) || !anchors.contains(QStringLiteral("-"))) {
            fprintf(stderr, "FAIL: OPN-9 - op-amp '+'/'-' anchors missing\n");
            failed++;
        }
    }

    if (failed == 0)
        fprintf(stderr, "PASS: path-op node name completion (op amp regression)\n");
    return failed;
}

// Regression (user report): commands defined with \let (\let\coord=\showcoord)
// were not offered in command completion because \let was never parsed.
static int test_let_command_completion()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\def\\showcoord(#1){coordinate(#1)}\n"
        "\\let\\coord=\\showcoord\n"
        "\\let\\coord2\\showcoord\n"
        "\\edef\\expcmd{expanded}\n"
        "\\gdef\\globcmd#1{#1}\n"
        "\\xdef\\gxcmd{global}\n"
        "\\begin{tikzpicture}\n\\end{tikzpicture}\n");
    TikzDocumentState state;
    state.reparse(&doc);

    const auto &cmds = state.userCommands();
    if (!cmds.contains("\\showcoord")) { fprintf(stderr, "FAIL: LET-1 - \\def\\showcoord not parsed\n"); failed++; }
    if (!cmds.contains("\\coord")) { fprintf(stderr, "FAIL: LET-2 - \\let\\coord not parsed\n"); failed++; }
    if (!cmds.contains("\\coord2")) { fprintf(stderr, "FAIL: LET-3 - \\let w/o '=' not parsed\n"); failed++; }
    if (!cmds.contains("\\expcmd")) { fprintf(stderr, "FAIL: LET-4 - \\edef not parsed\n"); failed++; }
    if (!cmds.contains("\\globcmd")) { fprintf(stderr, "FAIL: LET-5 - \\gdef not parsed\n"); failed++; }
    if (!cmds.contains("\\gxcmd")) { fprintf(stderr, "FAIL: LET-6 - \\xdef not parsed\n"); failed++; }

    // Verify the command completion model includes the let-defined command.
    QPlainTextEdit editor;
    editor.show();
    TikzCompleter completer(&editor);
    completer.setDocumentState(&state);
    completer.updateUserModels();

    const QStringList cmdsModel = completer.modelWordsForContext(TikzCompleter::TkzCtxCmd);
    if (!cmdsModel.contains(QStringLiteral("\\coord"))) {
        fprintf(stderr, "FAIL: LET-7 - '\\coord' not in command completion model\n");
        failed++;
    }
    if (!cmdsModel.contains(QStringLiteral("\\coord2"))) {
        fprintf(stderr, "FAIL: LET-8 - '\\coord2' not in command completion model\n");
        failed++;
    }
    if (!cmdsModel.contains(QStringLiteral("\\showcoord"))) {
        fprintf(stderr, "FAIL: LET-9 - '\\showcoord' not in command completion model\n");
        failed++;
    }

    if (failed == 0)
        fprintf(stderr, "PASS: \\let command completion\n");
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
    failed += test_arrow_case_variants_present();
    failed += test_colors_in_options();
    failed += test_value_hints_arc_angles();
    failed += test_value_hints_curve_angles();
    failed += test_value_hints_angles_library();
    failed += test_path_keywords_completable();
    failed += test_path_word_completion_triggers();
    failed += test_intersection_cs_after_value();
    failed += test_circuitikz_annotation_keys();
    failed += test_model_clearing_on_empty_list();
    failed += test_key_handlers();
    failed += test_eq_color_completion();
    failed += test_eq_key_name_extraction();
    failed += test_command_for_option_context();
    failed += test_multiline_context();
    failed += test_bracketed_brace_value_context();
    failed += test_no_bogus_commands();
    failed += test_usetikzlibrary_context();
    failed += test_circuitikz_components_accurate();
    failed += test_standard_tikz_no_bogus();
    failed += test_mathfunctions_accurate();
    failed += test_decorations_accurate();
    failed += test_decoration_eq_completion();
    failed += test_library_keys_present();
    failed += test_petri_library_gated();
    failed += test_physics_siunitx_gated();
    failed += test_pgfplots_keys_accurate();
    failed += test_package_completion_accurate();
    failed += test_path_word_set_restricted();
    failed += test_path_word_completion_correctness();
    failed += test_positioning_key_coord_completion();
    failed += test_intersections_coord_completion();
    failed += test_anchors_source_accurate();
    failed += test_anchor_library_gating();
    failed += test_circuitikz_pin_anchors();
    failed += test_label_position_completion();
    failed += test_name_path_in_node();
    failed += test_of_path_completion();
    failed += test_font_completion();
    failed += test_spaced_completion_accept_no_stray_char();
    failed += test_to_path_completion();
    failed += test_graphs_and_matrix_options();
    failed += test_coordinate_system_completion();
    failed += test_path_op_node_name_completion();
    failed += test_let_command_completion();

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll TikZ completer tests passed!\n");
    return 0;
}
