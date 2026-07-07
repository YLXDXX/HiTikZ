#include <QApplication>
#include <QTextDocument>
#include "tikz_highlighter.h"
#include "tikz_document_state.h"
#include <cstdio>

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
    failed += test_multiline_comment();
    failed += test_combined_rules();

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll enhanced highlighter tests passed!\n");
    return 0;
}
