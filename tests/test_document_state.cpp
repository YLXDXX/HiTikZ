#include <QApplication>
#include <QTextDocument>
#include <QTextCursor>
#include "tikz_document_state.h"
#include <cstdio>

static int test_scope_tracking()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\begin{tikzpicture}\n"
        "  \\draw (0,0) -- (1,1);\n"
        "  \\begin{scope}\n"
        "    \\draw (0,0) circle (1);\n"
        "  \\end{scope}\n"
        "  \\begin{axis}\n"
        "    \\addplot {x^2};\n"
        "  \\end{axis}\n"
        "\\end{tikzpicture}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    // Position 0: outside any tikz env
    {
        auto *s = state.currentScope(0);
        if (s) {
            fprintf(stderr, "FAIL: DCS-1 - Should have no scope at position 0\n");
            failed++;
        }
    }

    // Position ~20: inside tikzpicture
    {
        auto *s = state.currentScope(20);
        if (!s || s->env != "tikzpicture") {
            fprintf(stderr, "FAIL: DCS-2 - Should be in tikzpicture at pos 20 (got: %s)\n",
                    s ? qPrintable(s->env) : "null");
            failed++;
        }
    }

    // Position ~50: inside scope nested in tikzpicture
    {
        auto *s = state.currentScope(50);
        if (!s || s->env != "scope") {
            fprintf(stderr, "FAIL: DCS-3 - Should be in scope at pos 50 (got: %s)\n",
                    s ? qPrintable(s->env) : "null");
            failed++;
        }
    }

    // Position ~80: inside axis nested in tikzpicture
    {
        auto *s = state.currentScope(80);
        if (!s || s->env != "axis") {
            fprintf(stderr, "FAIL: DCS-4 - Should be in axis at pos 80 (got: %s)\n",
                    s ? qPrintable(s->env) : "null");
            failed++;
        }
    }

    // After \end{tikzpicture} (at end of doc)
    {
        auto *s = state.currentScope(doc.toPlainText().length());
        if (s) {
            fprintf(stderr, "FAIL: DCS-5 - Should have no scope after \\end{tikzpicture}\n");
            failed++;
        }
    }

    if (failed == 0) fprintf(stderr, "PASS: scope tracking\n");
    return failed;
}

static int test_library_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\usetikzlibrary{calc, positioning, shapes.geometric}\n"
        "\\begin{tikzpicture}\n"
        "  \\draw (0,0) -- (1,1);\n"
        "\\end{tikzpicture}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    const auto &libs = state.activeLibs();

    if (!libs.contains("calc")) {
        fprintf(stderr, "FAIL: DCS-L1 - Active libs should contain 'calc'\n");
        failed++;
    }
    if (!libs.contains("positioning")) {
        fprintf(stderr, "FAIL: DCS-L2 - Active libs should contain 'positioning'\n");
        failed++;
    }
    if (!libs.contains("shapes.geometric")) {
        fprintf(stderr, "FAIL: DCS-L3 - Active libs should contain 'shapes.geometric'\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: library parsing\n");
    return failed;
}

static int test_user_style_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\tikzset{\n"
        "  mybox/.style={draw, rectangle, fill=blue!20},\n"
        "  mycircle/.style={draw, circle, fill=red!20}\n"
        "}\n"
        "\\tikzstyle{mystyle}=[thick, dashed]\n"
        "\\begin{tikzpicture}\n"
        "  \\node[mybox] {text};\n"
        "\\end{tikzpicture}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    const auto &styles = state.userStyles();

    if (!styles.contains("mybox")) {
        fprintf(stderr, "FAIL: DCS-S1 - User styles should contain 'mybox'\n");
        failed++;
    }
    if (!styles.contains("mycircle")) {
        fprintf(stderr, "FAIL: DCS-S2 - User styles should contain 'mycircle'\n");
        failed++;
    }
    if (!styles.contains("mystyle")) {
        fprintf(stderr, "FAIL: DCS-S3 - User styles should contain 'mystyle'\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: user style parsing\n");
    return failed;
}

static int test_coordinate_node_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\coordinate (A) at (0,0);\n"
        "\\coordinate[red] (B) at (1,2);\n"
        "\\node (myNode) at (3,4) {label};\n"
        "\\node[draw] (otherNode) at (5,6) {text};\n");

    TikzDocumentState state;
    state.reparse(&doc);

    const auto &coords = state.userCoordinates();
    const auto &nodes = state.userNodes();

    if (!coords.contains("A")) {
        fprintf(stderr, "FAIL: DCS-C1 - Coords should contain 'A'\n");
        failed++;
    }
    if (!coords.contains("B")) {
        fprintf(stderr, "FAIL: DCS-C2 - Coords should contain 'B'\n");
        failed++;
    }
    if (!nodes.contains("myNode")) {
        fprintf(stderr, "FAIL: DCS-C3 - Nodes should contain 'myNode'\n");
        failed++;
    }
    if (!nodes.contains("otherNode")) {
        fprintf(stderr, "FAIL: DCS-C4 - Nodes should contain 'otherNode'\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: coordinate/node parsing\n");
    return failed;
}

static int test_foreach_variable_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\foreach \\x in {1,2,3} {\n"
        "  \\draw (\\x,0) -- (\\x,1);\n"
        "}\n"
        "\\foreach \\i/\\j in {a/b, c/d} {\n"
        "  \\node at (\\i,\\j) {};\n"
        "}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    const auto &vars = state.foreachVars();

    if (!vars.contains("x")) {
        fprintf(stderr, "FAIL: DCS-F1 - Foreach vars should contain 'x'\n");
        failed++;
    }
    if (!vars.contains("i")) {
        fprintf(stderr, "FAIL: DCS-F2 - Foreach vars should contain 'i'\n");
        failed++;
    }
    if (!vars.contains("j")) {
        fprintf(stderr, "FAIL: DCS-F3 - Foreach vars should contain 'j'\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: foreach variable parsing\n");
    return failed;
}

static int test_color_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\definecolor{myblue}{RGB}{0,0,255}\n"
        "\\definecolor{myred}{HTML}{FF0000}\n"
        "\\colorlet{mygrey}{gray!50}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    const auto &colors = state.definedColors();

    if (!colors.contains("myblue")) {
        fprintf(stderr, "FAIL: DCS-K1 - Colors should contain 'myblue'\n");
        failed++;
    }
    if (!colors.contains("myred")) {
        fprintf(stderr, "FAIL: DCS-K2 - Colors should contain 'myred'\n");
        failed++;
    }
    if (!colors.contains("mygrey")) {
        fprintf(stderr, "FAIL: DCS-K3 - Colors should contain 'mygrey'\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: color parsing\n");
    return failed;
}

static int test_user_command_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\newcommand{\\mycmd}{content}\n"
        "\\renewcommand{\\oldcmd}[2]{#1 #2}\n"
        "\\def\\myfunc#1{#1}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    const auto &cmds = state.userCommands();

    if (!cmds.contains("\\mycmd")) {
        fprintf(stderr, "FAIL: DCS-U1 - User commands should contain '\\mycmd'\n");
        failed++;
    }
    if (!cmds.contains("\\oldcmd")) {
        fprintf(stderr, "FAIL: DCS-U2 - User commands should contain '\\oldcmd'\n");
        failed++;
    }
    if (!cmds.contains("\\myfunc")) {
        fprintf(stderr, "FAIL: DCS-U3 - User commands should contain '\\myfunc'\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: user command parsing\n");
    return failed;
}

static int test_current_env_name()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\begin{tikzpicture}\n"   // pos 0-21
        "  \\draw (0,0) -- (1,1);\n"
        "  \\begin{scope}[thick]\n"  // ~42
        "    \\draw (0,0) circle (1);\n"
        "  \\end{scope}\n"           // ~76
        "\\end{tikzpicture}\n"       // ~92
        "outside text\n");

    TikzDocumentState state;
    state.reparse(&doc);

    // Inside tikzpicture (pos 30)
    if (state.currentEnvName(30) != "tikzpicture") {
        fprintf(stderr, "FAIL: DCS-E1 - pos 30 should be 'tikzpicture' (got: %s)\n",
                qPrintable(state.currentEnvName(30)));
        failed++;
    }

    // Inside scope (pos 55)
    if (state.currentEnvName(55) != "scope") {
        fprintf(stderr, "FAIL: DCS-E2 - pos 55 should be 'scope' (got: %s)\n",
                qPrintable(state.currentEnvName(55)));
        failed++;
    }

    // Outside (pos 100)
    if (!state.currentEnvName(100).isEmpty()) {
        fprintf(stderr, "FAIL: DCS-E3 - pos 100 should be empty (got: %s)\n",
                qPrintable(state.currentEnvName(100)));
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: current environment name\n");
    return failed;
}

static int test_snippet_libraries()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText("\\begin{tikzpicture}\n\\draw (0,0) -- (1,1);\n\\end{tikzpicture}");

    TikzDocumentState state;
    state.setSnippetLibraries({"calc", "shapes.geometric"});
    state.reparse(&doc);

    const auto &libs = state.activeLibs();
    if (!libs.contains("calc")) {
        fprintf(stderr, "FAIL: DCS-SL1 - Snippet lib 'calc' should be active\n");
        failed++;
    }
    if (!libs.contains("shapes.geometric")) {
        fprintf(stderr, "FAIL: DCS-SL2 - Snippet lib 'shapes.geometric' should be active\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: snippet libraries\n");
    return failed;
}

static int test_circuitikz_scope()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\begin{circuitikz}\n"
        "  \\draw (0,0) to[R] (2,0);\n"
        "\\end{circuitikz}\n");

    TikzDocumentState state;
    state.reparse(&doc);

    if (state.currentEnvName(10) != "circuitikz") {
        fprintf(stderr, "FAIL: DCS-CK1 - pos 10 should be 'circuitikz' (got: %s)\n",
                qPrintable(state.currentEnvName(10)));
        failed++;
    }

    const auto *s = state.currentScope(10);
    if (!s || s->env != "circuitikz") {
        fprintf(stderr, "FAIL: DCS-CK2 - pos 10 should have circuitikz scope\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: circuitikz scope\n");
    return failed;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int failed = 0;

    failed += test_scope_tracking();
    failed += test_library_parsing();
    failed += test_user_style_parsing();
    failed += test_coordinate_node_parsing();
    failed += test_foreach_variable_parsing();
    failed += test_color_parsing();
    failed += test_user_command_parsing();
    failed += test_current_env_name();
    failed += test_snippet_libraries();
    failed += test_circuitikz_scope();

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll TikzDocumentState tests passed!\n");
    return 0;
}
