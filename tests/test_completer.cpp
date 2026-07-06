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
        "help lines",
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
        {QStringLiteral("\\draw["), TikzCompleter::TkzCtxBrk, "open bracket"},
        {QStringLiteral("\\draw[help lines"), TikzCompleter::TkzCtxBrk, "help lines option"},
        {QStringLiteral("\\draw[help lin"), TikzCompleter::TkzCtxBrk, "partial help lines"},
        {QStringLiteral("\\draw[->"), TikzCompleter::TkzCtxBrk, "arrow spec in bracket"},
        {QStringLiteral("\\draw[red, thick, ->"), TikzCompleter::TkzCtxBrk, "multiple bracket opts"},
        {QStringLiteral("\\draw[->,>="), TikzCompleter::TkzCtxEq, ">= with = at end triggers Eq"},
        {QStringLiteral("\\draw[color="), TikzCompleter::TkzCtxEq, "after = in bracket"},
        {QStringLiteral("\\draw[color=red"), TikzCompleter::TkzCtxBrk, "color value completed"},
        {QStringLiteral("\\draw[->,>=stealth"), TikzCompleter::TkzCtxBrk, ">= arrow spec completed"},
        {QStringLiteral("\\draw[>=st"), TikzCompleter::TkzCtxBrk, ">=stealth typing"},
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

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll TikZ completer tests passed!\n");
    return 0;
}
