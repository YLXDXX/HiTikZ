#include <QApplication>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>
#include <QColor>
#include "tikz_highlighter.h"
#include "tikz_document_state.h"
#include <cstdio>

// Returns true if any applied format range covering `pos` in the given block
// uses `color` as its foreground (used to detect specific highlight colors).
static bool hasForegroundAt(const QTextDocument &doc, int blockNumber, int pos,
                            const QColor &color)
{
    QTextBlock block = doc.findBlockByNumber(blockNumber);
    if (!block.isValid() || !block.layout()) return false;
    const auto formats = block.layout()->formats();
    for (const auto &fr : formats) {
        if (pos >= fr.start && pos < fr.start + fr.length
            && fr.format.foreground().color() == color)
            return true;
    }
    return false;
}


static int test_basic_rules()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText("\\draw[red, thick] (0,0) -- (1,1); % comment");

    TikzHighlighter hl(&doc);
    // Just verify it doesn't crash
    Q_UNUSED(hl);

    fprintf(stderr, "PASS: basic rules don't crash\n");
    return failed;
}

static int test_pgf_path_rule()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText("/tikz/every picture/.style={}");

    TikzHighlighter hl(&doc);
    // Verify the highlighter handles PGF paths
    Q_UNUSED(hl);

    fprintf(stderr, "PASS: PGF path rule doesn't crash\n");
    return failed;
}

static int test_handler_rule()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\tikzset{\n"
        "  mykey/.style={draw, fill=red},\n"
        "  other/.code={something},\n"
        "  third/.default=value\n"
        "}");

    TikzHighlighter hl(&doc);
    Q_UNUSED(hl);

    fprintf(stderr, "PASS: handler rule doesn't crash\n");
    return failed;
}

static int test_library_rule()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText("\\usetikzlibrary{calc, positioning, shapes.geometric}");

    TikzHighlighter hl(&doc);
    Q_UNUSED(hl);

    fprintf(stderr, "PASS: library rule doesn't crash\n");
    return failed;
}

static int test_user_style_highlight()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\tikzset{mystyle/.style={draw, thick}}\n"
        "\\begin{tikzpicture}\n"
        "  \\node[mystyle] {text};\n"
        "\\end{tikzpicture}");

    TikzDocumentState state;
    state.reparse(&doc);

    TikzHighlighter hl(&doc);
    hl.setDocumentState(&state);

    // Verify the state has the style
    if (!state.userStyles().contains("mystyle")) {
        fprintf(stderr, "FAIL: HL-U1 - mystyle should be in user styles\n");
        failed++;
    }

    fprintf(stderr, "PASS: user style highlight\n");
    return failed;
}

static int test_user_node_highlight()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\begin{tikzpicture}\n"
        "  \\node (A) at (0,0) {text};\n"
        "  \\draw (A) -- (1,1);\n"
        "\\end{tikzpicture}");

    TikzDocumentState state;
    state.reparse(&doc);

    TikzHighlighter hl(&doc);
    hl.setDocumentState(&state);

    if (!state.userNodes().contains("A")) {
        fprintf(stderr, "FAIL: HL-N1 - A should be in user nodes\n");
        failed++;
    }

    fprintf(stderr, "PASS: user node highlight\n");
    return failed;
}

static int test_foreach_var_highlight()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\foreach \\x in {1,2,3} {\n"
        "  \\draw (\\x,0) -- (\\x,1);\n"
        "}");

    TikzDocumentState state;
    state.reparse(&doc);

    TikzHighlighter hl(&doc);
    hl.setDocumentState(&state);

    if (!state.foreachVars().contains("x")) {
        fprintf(stderr, "FAIL: HL-F1 - x should be in foreach vars\n");
        failed++;
    }

    fprintf(stderr, "PASS: foreach var highlight\n");
    return failed;
}

static int test_key_value_highlight()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText("\\draw[color=red, thick, draw=blue] (0,0) -- (1,1);");

    TikzHighlighter hl(&doc);
    // Just verify it doesn't crash
    Q_UNUSED(hl);

    fprintf(stderr, "PASS: key=value highlight\n");
    return failed;
}

// key=value highlighting must ignore '=' nested inside a value's braces.
static int test_key_value_brace_depth()
{
    int failed = 0;
    const QColor keyColor(20, 100, 100); // TikzHighlighter m_keyFormat foreground

    QTextDocument doc;
    doc.setPlainText(
        "\\draw[color=red, name/.style={inner=blue}] (0,0);");
    TikzHighlighter hl(&doc);
    hl.rehighlight();

    const QString text = doc.firstBlock().text();

    int colorPos = text.indexOf(QStringLiteral("color="));
    if (colorPos < 0 || !hasForegroundAt(doc, 0, colorPos, keyColor)) {
        fprintf(stderr, "FAIL: KVB-1 - top-level key 'color' should use the key color\n");
        failed++;
    }

    int innerPos = text.indexOf(QStringLiteral("inner="));
    if (innerPos >= 0 && hasForegroundAt(doc, 0, innerPos, keyColor)) {
        fprintf(stderr, "FAIL: KVB-2 - brace-nested '=' must not highlight 'inner' as a key\n");
        failed++;
    }

    fprintf(stderr, "%s: key=value respects brace depth\n", failed == 0 ? "PASS" : "FAIL");
    return failed;
}

// Options split across multiple lines must stay option-colored on every line
// (the closing ']' ends the option list; content after it must not).
static int test_multiline_option_highlight()
{
    int failed = 0;
    const QColor optColor(0, 120, 120); // TikzHighlighter m_optionFormat foreground

    QTextDocument doc;
    doc.setPlainText(
        "\\draw[red,\n"
        "      line width=2pt,\n"
        "      ->] (0,0) -- (1,1);");
    TikzHighlighter hl(&doc);
    hl.rehighlight();

    // Continuation line 1: "line" must be option-colored.
    QString b1 = doc.findBlockByNumber(1).text();
    int linePos = b1.indexOf(QStringLiteral("line"));
    if (linePos < 0 || !hasForegroundAt(doc, 1, linePos, optColor)) {
        fprintf(stderr, "FAIL: MLO-1 - multi-line option continuation should be option-colored\n");
        failed++;
    }

    // Continuation line 2: the "->" before the closing ']' stays option-colored.
    QString b2 = doc.findBlockByNumber(2).text();
    int arrowPos = b2.indexOf(QStringLiteral("->"));
    if (arrowPos < 0 || !hasForegroundAt(doc, 2, arrowPos, optColor)) {
        fprintf(stderr, "FAIL: MLO-2 - option content before closing ']' should be option-colored\n");
        failed++;
    }

    // After the closing ']', the coordinate must NOT be option-colored.
    int coordPos = b2.indexOf(QStringLiteral("(0,0)"));
    if (coordPos >= 0 && hasForegroundAt(doc, 2, coordPos, optColor)) {
        fprintf(stderr, "FAIL: MLO-3 - content after ']' must not be option-colored\n");
        failed++;
    }

    fprintf(stderr, "%s: multi-line option bracket highlighting\n", failed == 0 ? "PASS" : "FAIL");
    return failed;
}

static int test_multiline_comment()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\begin{comment}\n"
        "  this is a comment\n"
        "\\end{comment}\n"
        "not a comment");

    TikzHighlighter hl(&doc);
    Q_UNUSED(hl);

    fprintf(stderr, "PASS: multiline comment\n");
    return failed;
}

static int test_combined_rules()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\usetikzlibrary{calc}\n"
        "\\tikzset{mystyle/.style={draw, thick}}\n"
        "\\definecolor{myblue}{RGB}{0,0,255}\n"
        "\\newcommand{\\mycmd}{hello}\n"
        "\\begin{tikzpicture}\n"
        "  \\coordinate (A) at (0,0);\n"
        "  \\foreach \\x in {1,2,3} {\n"
        "    \\node[mystyle, draw=red] at (\\x,0) {\\x};\n"
        "  }\n"
        "  \\draw[->, color=myblue] (A) -- (3,0);\n"
        "\\end{tikzpicture}");

    TikzDocumentState state;
    state.reparse(&doc);

    TikzHighlighter hl(&doc);
    hl.setDocumentState(&state);

    // Verify state tracking
    if (state.activeLibs().size() == 0) {
        fprintf(stderr, "FAIL: HL-C1 - calc library should be active\n");
        failed++;
    }
    if (!state.userStyles().contains("mystyle")) {
        fprintf(stderr, "FAIL: HL-C2 - mystyle should be in user styles\n");
        failed++;
    }
    if (!state.userCoordinates().contains("A")) {
        fprintf(stderr, "FAIL: HL-C3 - A should be in coords\n");
        failed++;
    }
    if (!state.foreachVars().contains("x")) {
        fprintf(stderr, "FAIL: HL-C4 - x should be in foreach vars\n");
        failed++;
    }
    if (!state.userCommands().contains("\\mycmd")) {
        fprintf(stderr, "FAIL: HL-C5 - \\mycmd should be in user commands\n");
        failed++;
    }
    if (!state.definedColors().contains("myblue")) {
        fprintf(stderr, "FAIL: HL-C6 - myblue should be in defined colors\n");
        failed++;
    }

    fprintf(stderr, "PASS: combined rules\n");
    return failed;
}

static int test_foreach_vars_with_spaces()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\foreach \\xyz / \\xtext in {-1,-0.5/-\\frac{1}{2},0.5/\\frac{1}{2},1} {\n"
        "  \\draw (\\xyz,1pt) -- (\\xyz,-1pt) node { $\\xtext$ } ;\n"
        "}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    const auto &vars = state.foreachVars();
    if (!vars.contains("xyz")) {
        fprintf(stderr, "FAIL: HL-FS1 - xyz should be in foreach vars\n");
        failed++;
    }
    if (!vars.contains("xtext")) {
        fprintf(stderr, "FAIL: HL-FS2 - xtext should be in foreach vars\n");
        failed++;
    }

    TikzHighlighter hl(&doc);
    hl.setDocumentState(&state);
    hl.rehighlight();

    fprintf(stderr, "PASS: foreach vars with spaces around /\n");
    return failed;
}

static bool isCommentFormat(const QTextCharFormat &fmt)
{
    return fmt.foreground().color() == QColor(150, 150, 150);
}

static int test_comment_not_overwritten_by_user_highlights()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\tikzset{mystyle/.style={draw}}\n"
        "\\coordinate (A) at (0,0);\n"
        "\\foreach \\x in {1,2,3} {\n"
        "  \\draw (\\x,0) -- (\\x,1); % mystyle A \\x here\n"
        "}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    TikzHighlighter hl(&doc);
    hl.setDocumentState(&state);
    hl.rehighlight();

    QTextBlock block = doc.findBlockByNumber(3);
    QTextLayout *layout = block.layout();

    QVector<QTextLayout::FormatRange> ranges = layout->formats();

    int commentStart = block.text().indexOf('%');

    for (const auto &r : ranges) {
        if (r.start >= commentStart && commentStart > 0) {
            if (!isCommentFormat(r.format)) {
                if (r.format.foreground() != QColor(150, 150, 150)) {
                    fprintf(stderr, "FAIL: HL-CP1 - format at %d (inside comment) should be comment format, got R=%d G=%d B=%d\n",
                            r.start,
                            r.format.foreground().color().red(),
                            r.format.foreground().color().green(),
                            r.format.foreground().color().blue());
                    failed++;
                    break;
                }
            }
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: comment not overwritten by user highlights\n");
    return failed;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int failed = 0;

    failed += test_basic_rules();
    failed += test_pgf_path_rule();
    failed += test_handler_rule();
    failed += test_library_rule();
    failed += test_user_style_highlight();
    failed += test_user_node_highlight();
    failed += test_foreach_var_highlight();
    failed += test_key_value_highlight();
    failed += test_key_value_brace_depth();
    failed += test_multiline_option_highlight();
    failed += test_multiline_comment();
    failed += test_combined_rules();
    failed += test_foreach_vars_with_spaces();
    failed += test_comment_not_overwritten_by_user_highlights();

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll enhanced highlighter tests passed!\n");
    return 0;
}
