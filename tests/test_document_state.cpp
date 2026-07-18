#include <QApplication>
#include <QTextDocument>
#include <QTextBlock>
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

    // Compute block positions for testing
    int line1 = doc.findBlockByNumber(1).position() + 3;
    int line3 = doc.findBlockByNumber(3).position() + 3;  // inside scope body
    int line6 = doc.findBlockByNumber(6).position() + 3;  // inside axis body

    if (!state.currentScope(line1) || state.currentScope(line1)->env != "tikzpicture") {
        fprintf(stderr, "FAIL: DCS-1 - line 1 should be in tikzpicture\n");
        failed++;
    }
    if (!state.currentScope(line3) || state.currentScope(line3)->env != "scope") {
        fprintf(stderr, "FAIL: DCS-2 - line 3 should be in scope (got: %s)\n",
                state.currentScope(line3) ? qPrintable(state.currentScope(line3)->env) : "null");
        failed++;
    }
    if (!state.currentScope(line6) || state.currentScope(line6)->env != "axis") {
        fprintf(stderr, "FAIL: DCS-3 - line6 should be in axis (got: %s)\n",
                state.currentScope(line6) ? qPrintable(state.currentScope(line6)->env) : "null");
        failed++;
    }

    // Outside (after last line)
    if (state.currentScope(doc.characterCount())) {
        fprintf(stderr, "FAIL: DCS-4 - after end should have no scope\n");
        failed++;
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
        "\\begin{tikzpicture}\n  \\draw (0,0) -- (1,1);\n\\end{tikzpicture}\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &libs = state.activeLibs();
    if (!libs.contains("calc")) { fprintf(stderr, "FAIL: DCS-L1\n"); failed++; }
    if (!libs.contains("positioning")) { fprintf(stderr, "FAIL: DCS-L2\n"); failed++; }
    if (!libs.contains("shapes.geometric")) { fprintf(stderr, "FAIL: DCS-L3\n"); failed++; }
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
        "\\tikzstyle{mystyle}=[thick, dashed]\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &styles = state.userStyles();
    if (!styles.contains("mybox")) { fprintf(stderr, "FAIL: DCS-S1\n"); failed++; }
    if (!styles.contains("mycircle")) { fprintf(stderr, "FAIL: DCS-S2\n"); failed++; }
    if (!styles.contains("mystyle")) { fprintf(stderr, "FAIL: DCS-S3\n"); failed++; }
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
        "\\node[draw] (otherNode) at (5,6) {text};\n"
        // Option list contains ']' nested inside a brace group (label={[blue]..}).
        // The name after such an option must still be extracted.
        "\\coordinate [label={[blue]right:$B$}] (tempB) at ($ (1.5,0.5)+0.1*(rand,rand) $);\n"
        "\\node[pin={[red]above:x}] (pinNode) at (7,8) {n};\n");
    TikzDocumentState state;
    state.reparse(&doc);
    if (!state.userCoordinates().contains("A")) { fprintf(stderr, "FAIL: DCS-C1\n"); failed++; }
    if (!state.userCoordinates().contains("B")) { fprintf(stderr, "FAIL: DCS-C2\n"); failed++; }
    if (!state.userNodes().contains("myNode")) { fprintf(stderr, "FAIL: DCS-C3\n"); failed++; }
    if (!state.userNodes().contains("otherNode")) { fprintf(stderr, "FAIL: DCS-C4\n"); failed++; }
    if (!state.userCoordinates().contains("tempB")) { fprintf(stderr, "FAIL: DCS-C5 (coord name after label={[..]..} option)\n"); failed++; }
    if (!state.userNodes().contains("pinNode")) { fprintf(stderr, "FAIL: DCS-C6 (node name after pin={[..]..} option)\n"); failed++; }
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
    if (!vars.contains("x")) { fprintf(stderr, "FAIL: DCS-F1\n"); failed++; }
    if (!vars.contains("i")) { fprintf(stderr, "FAIL: DCS-F2\n"); failed++; }
    if (!vars.contains("j")) { fprintf(stderr, "FAIL: DCS-F3\n"); failed++; }
    if (failed == 0) fprintf(stderr, "PASS: foreach variable parsing\n");
    return failed;
}

static int test_foreach_spaces_around_slash()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\foreach \\xyz / \\xtext in {-1,-0.5/-\\frac{1}{2},0.5/\\frac{1}{2},1} {\n"
        "  \\draw (\\xyz,1pt) -- (\\xyz,-1pt) node [anchor=north,fill=white] { $\\xtext$ } ;\n"
        "}\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &vars = state.foreachVars();
    if (!vars.contains("xyz")) {
        fprintf(stderr, "FAIL: DCS-FS1 - 'xyz' should be in foreach vars (space before /)\n");
        failed++;
    }
    if (!vars.contains("xtext")) {
        fprintf(stderr, "FAIL: DCS-FS2 - 'xtext' should be in foreach vars (space after /)\n");
        failed++;
    }
    if (failed == 0) fprintf(stderr, "PASS: foreach with spaces around /\n");
    return failed;
}

static int test_foreach_multiple_vars()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\foreach \\a/\\b/\\c in {1/2/3, 4/5/6} {\n"
        "  \\node at (\\a,\\b) {\\c};\n"
        "}\n"
        "\\foreach \\p / \\q / \\r in {x/y/z} { (\\p,\\q) -- (\\r,0); }\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &vars = state.foreachVars();
    if (!vars.contains("a")) { fprintf(stderr, "FAIL: DCS-FM1 - 'a' should be in foreach vars\n"); failed++; }
    if (!vars.contains("b")) { fprintf(stderr, "FAIL: DCS-FM2 - 'b' should be in foreach vars\n"); failed++; }
    if (!vars.contains("c")) { fprintf(stderr, "FAIL: DCS-FM3 - 'c' should be in foreach vars\n"); failed++; }
    if (!vars.contains("p")) { fprintf(stderr, "FAIL: DCS-FM4 - 'p' should be in foreach vars\n"); failed++; }
    if (!vars.contains("q")) { fprintf(stderr, "FAIL: DCS-FM5 - 'q' should be in foreach vars\n"); failed++; }
    if (!vars.contains("r")) { fprintf(stderr, "FAIL: DCS-FM6 - 'r' should be in foreach vars\n"); failed++; }
    if (failed == 0) fprintf(stderr, "PASS: foreach with multiple variables\n");
    return failed;
}

static int test_foreach_optional_bracket()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\foreach \\x [count=\\i] in {a,b,c} {\n"
        "  \\foreach \\y / \\z [evaluate=\\y as \\yy using int(\\y+1)] in {1/2,3/4} { }\n"
        "}\n");
    TikzDocumentState state;
    state.reparse(&doc);
    if (!state.foreachVars().contains("x")) { fprintf(stderr, "FAIL: FOB-1\n"); failed++; }
    if (!state.foreachVars().contains("i")) { fprintf(stderr, "FAIL: FOB-2\n"); failed++; }
    if (!state.foreachVars().contains("y")) { fprintf(stderr, "FAIL: FOB-3\n"); failed++; }
    if (!state.foreachVars().contains("z")) { fprintf(stderr, "FAIL: FOB-4\n"); failed++; }
    if (!state.foreachVars().contains("yy")) { fprintf(stderr, "FAIL: FOB-5\n"); failed++; }
    if (failed == 0) fprintf(stderr, "PASS: foreach optional bracket support\n");
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
    if (!colors.contains("myblue")) { fprintf(stderr, "FAIL: DCS-K1\n"); failed++; }
    if (!colors.contains("myred")) { fprintf(stderr, "FAIL: DCS-K2\n"); failed++; }
    if (!colors.contains("mygrey")) { fprintf(stderr, "FAIL: DCS-K3\n"); failed++; }
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
    if (!cmds.contains("\\mycmd")) { fprintf(stderr, "FAIL: DCS-U1\n"); failed++; }
    if (!cmds.contains("\\oldcmd")) { fprintf(stderr, "FAIL: DCS-U2\n"); failed++; }
    if (!cmds.contains("\\myfunc")) { fprintf(stderr, "FAIL: DCS-U3\n"); failed++; }
    if (failed == 0) fprintf(stderr, "PASS: user command parsing\n");
    return failed;
}

static int test_current_env_name()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\begin{tikzpicture}\n"
        "  \\draw (0,0) -- (1,1);\n"
        "  \\begin{scope}[thick]\n"
        "    \\draw (0,0) circle (1);\n"
        "  \\end{scope}\n"
        "\\end{tikzpicture}\n"
        "outside text\n");
    TikzDocumentState state;
    state.reparse(&doc);
    int line1 = doc.findBlockByNumber(1).position() + 3;
    int line3 = doc.findBlockByNumber(3).position() + 3;
    int afterText = doc.toPlainText().length();
    if (state.currentEnvName(line1) != "tikzpicture") {
        fprintf(stderr, "FAIL: DCS-E1 - expected tikzpicture, got %s\n", qPrintable(state.currentEnvName(line1)));
        failed++;
    }
    if (state.currentEnvName(line3) != "scope") {
        fprintf(stderr, "FAIL: DCS-E2 - expected scope, got %s\n", qPrintable(state.currentEnvName(line3)));
        failed++;
    }
    if (!state.currentEnvName(afterText - 1).isEmpty()) {
        fprintf(stderr, "FAIL: DCS-E3 - expected empty for outside text\n");
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
    if (!state.activeLibs().contains("calc")) { fprintf(stderr, "FAIL: DCS-SL1\n"); failed++; }
    if (!state.activeLibs().contains("shapes.geometric")) { fprintf(stderr, "FAIL: DCS-SL2\n"); failed++; }
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
    int insidePos = doc.findBlockByNumber(1).position() + 3;
    if (state.currentEnvName(insidePos) != "circuitikz") {
        fprintf(stderr, "FAIL: DCS-CK1 - expected circuitikz, got %s\n", qPrintable(state.currentEnvName(insidePos)));
        failed++;
    }
    if (!state.currentScope(insidePos) || state.currentScope(insidePos)->env != "circuitikz") {
        fprintf(stderr, "FAIL: DCS-CK2\n");
        failed++;
    }
    if (failed == 0) fprintf(stderr, "PASS: circuitikz scope\n");
    return failed;
}

static int test_style_with_spaces()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\tikzset{\n"
        "  test lines/.style={color=#1!20,very thin},\n"
        "  test lines/.default=red,\n"
        "  my box/.style={draw, thick},\n"
        "}\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &styles = state.userStyles();
    if (!styles.contains("test lines")) {
        fprintf(stderr, "FAIL: DCS-SW1 - 'test lines' with space should be detected\n");
        failed++;
    }
    if (!styles.contains("my box")) {
        fprintf(stderr, "FAIL: DCS-SW2 - 'my box' with space should be detected\n");
        failed++;
    }
    if (failed == 0) fprintf(stderr, "PASS: style names with spaces\n");
    return failed;
}

static int test_pic_name_detection()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\pic (alpha) [angle radius=0.5cm] {right angle = A--B--C};\n"
        "\\pic[red] (beta) at (0,0) {mypic};\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &nodes = state.userNodes();
    if (!nodes.contains("alpha")) {
        fprintf(stderr, "FAIL: DCS-PN1 - 'alpha' from \\pic should be detected\n");
        failed++;
    }
    if (!nodes.contains("beta")) {
        fprintf(stderr, "FAIL: DCS-PN2 - 'beta' from \\pic should be detected\n");
        failed++;
    }
    if (failed == 0) fprintf(stderr, "PASS: \\pic name detection\n");
    return failed;
}

// Verifies \usepackage{physics,siunitx,pgfplots} and the snippet packages field
// activate the corresponding completion libraries so package-gated commands show.
static int test_usepackage_activates_libs()
{
    int failed = 0;
    // via \usepackage in the document body
    QTextDocument doc;
    doc.setPlainText(
        "\\usepackage{physics}\n"
        "\\usepackage[per-mode=symbol]{siunitx}\n"
        "\\usepackage{pgfplots}\n"
        "\\begin{tikzpicture}\n\\end{tikzpicture}\n");
    TikzDocumentState state;
    state.reparse(&doc);
    for (const char *lib : { "physics", "siunitx", "pgfplots" }) {
        if (!state.activeLibs().contains(QString::fromUtf8(lib))) {
            fprintf(stderr, "FAIL: DCS-UP1 - \\usepackage should activate '%s'\n", lib);
            failed++;
        }
    }
    // chemfig / tikz-feynman activation
    QTextDocument doc3;
    doc3.setPlainText(
        "\\usepackage{chemfig}\n"
        "\\usepackage{tikz-feynman}\n");
    TikzDocumentState state3;
    state3.reparse(&doc3);
    for (const char *lib : { "chemfig", "tikz-feynman" }) {
        if (!state3.activeLibs().contains(QString::fromUtf8(lib))) {
            fprintf(stderr, "FAIL: DCS-UP3 - \\usepackage should activate '%s'\n", lib);
            failed++;
        }
    }
    // via the snippet packages metadata field
    TikzDocumentState state2;
    state2.setSnippetPackages({QStringLiteral("physics"), QStringLiteral("siunitx")});
    QTextDocument doc2;
    doc2.setPlainText("\\begin{tikzpicture}\n\\end{tikzpicture}\n");
    state2.reparse(&doc2);
    if (!state2.activeLibs().contains(QStringLiteral("physics"))
        || !state2.activeLibs().contains(QStringLiteral("siunitx"))) {
        fprintf(stderr, "FAIL: DCS-UP2 - snippet packages field should activate physics/siunitx\n");
        failed++;
    }
    if (failed == 0) fprintf(stderr, "PASS: \\usepackage activates completion libs\n");
    return failed;
}

// Regression: nodes that use name=<name> in their options (instead of the
// explicit (name) syntax) must still be extracted so the completer can offer
// them. E.g. \node[place,name=critical 1] [below=of ...] {};
static int test_node_name_in_options()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\node[place,name=critical 1] [below=of waiting 1] {};\n"
        "\\node[draw,name=my node] at (0,0) {hello};\n"
        "\\node[circle,name=Node3] {};\n"
        "\\pic[red,name={braced name}] at (1,1) {mypic};\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &nodes = state.userNodes();

    // Names from name=... in options must be extracted.
    if (!nodes.contains("critical 1")) {
        fprintf(stderr, "FAIL: DCS-NN1 - 'critical 1' via name= should be extracted\n");
        failed++;
    }
    if (!nodes.contains("my node")) {
        fprintf(stderr, "FAIL: DCS-NN2 - 'my node' via name= should be extracted\n");
        failed++;
    }
    if (!nodes.contains("Node3")) {
        fprintf(stderr, "FAIL: DCS-NN3 - 'Node3' via name= should be extracted\n");
        failed++;
    }
    // Brace-quoted name
    if (!nodes.contains("braced name")) {
        fprintf(stderr, "FAIL: DCS-NN4 - 'braced name' via name={...} should be extracted\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: node name extracted from name=... in options\n");
    return failed;
}

// Regression: nodes with both (name) and name=... should not duplicate.
static int test_node_both_name_styles()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\node[red] (A) at (0,0) {};\n"
        "\\node[name=B] (C) at (0,0) {};\n"
        "\\coordinate[name=D] (E) at (0,0);\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &nodes = state.userNodes();

    if (!nodes.contains("A")) { fprintf(stderr, "FAIL: DCS-BN1\n"); failed++; }
    if (!nodes.contains("B")) { fprintf(stderr, "FAIL: DCS-BN2\n"); failed++; }
    if (!nodes.contains("C")) { fprintf(stderr, "FAIL: DCS-BN3\n"); failed++; }
    if (!nodes.contains("D")) { fprintf(stderr, "FAIL: DCS-BN4\n"); failed++; }
    if (!nodes.contains("E")) { fprintf(stderr, "FAIL: DCS-BN5\n"); failed++; }

    if (failed == 0) fprintf(stderr, "PASS: both (name) and name= styles coexist\n");
    return failed;
}

// Feature 5: named paths declared via name path / name path global / name path
// local must be extracted so the 'of=' argument of name intersections can offer
// them. Covers brace-wrapped names, bare names, and names with '--' and spaces.
static int test_name_path_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\draw [name path=D--F] (0,0) -- (2,2);\n"
        "\\node [draw,circle through=(C),name path=circle H] (H) at (1,1) {};\n"
        "\\path [name path global=global line] (0,0) -- (1,0);\n"
        "\\path [name path={circle K}] (0,0) circle (1);\n"
        "\\path [name path local=localp] (0,0) -- (1,1);\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &paths = state.userPaths();
    if (!paths.contains("D--F")) { fprintf(stderr, "FAIL: DCS-NP1 - 'D--F' name path missing\n"); failed++; }
    if (!paths.contains("circle H")) { fprintf(stderr, "FAIL: DCS-NP2 - 'circle H' name path (in \\node) missing\n"); failed++; }
    if (!paths.contains("global line")) { fprintf(stderr, "FAIL: DCS-NP3 - 'global line' name path global missing\n"); failed++; }
    if (!paths.contains("circle K")) { fprintf(stderr, "FAIL: DCS-NP4 - 'circle K' brace-wrapped name path missing\n"); failed++; }
    if (!paths.contains("localp")) { fprintf(stderr, "FAIL: DCS-NP5 - 'localp' name path local missing\n"); failed++; }
    if (failed == 0) fprintf(stderr, "PASS: name path parsing (Feature 5)\n");
    return failed;
}

// Feature 6: coordinate names declared via by={...} inside name intersections
// must be extracted (with optional [options] prefixes stripped) so they are
// offered as coordinate completions.
static int test_by_coordinate_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\path [name intersections={of=D--F and circle K,by={[label=95:$L$]L,H}}];\n"
        "\\path [name intersections={of=a and b,by={P,Q,R}}];\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &coords = state.userCoordinates();
    if (!coords.contains("L")) { fprintf(stderr, "FAIL: DCS-BY1 - by= coord 'L' (with [..] prefix) missing\n"); failed++; }
    if (!coords.contains("H")) { fprintf(stderr, "FAIL: DCS-BY2 - by= coord 'H' missing\n"); failed++; }
    if (!coords.contains("P")) { fprintf(stderr, "FAIL: DCS-BY3 - by= coord 'P' missing\n"); failed++; }
    if (!coords.contains("Q")) { fprintf(stderr, "FAIL: DCS-BY4 - by= coord 'Q' missing\n"); failed++; }
    if (!coords.contains("R")) { fprintf(stderr, "FAIL: DCS-BY5 - by= coord 'R' missing\n"); failed++; }
    // The label option content ('95:$L$') must NOT leak in as a coordinate.
    if (coords.contains("95:$L$") || coords.contains("[label=95:$L$]L")) {
        fprintf(stderr, "FAIL: DCS-BY6 - option prefix leaked into coordinate name\n");
        failed++;
    }
    if (failed == 0) fprintf(stderr, "PASS: by= coordinate parsing (Feature 6)\n");
    return failed;
}

// The path-operation coordinate form 'coordinate (name)' (no backslash), used
// inside a \draw/\path, must be extracted for highlight + completion.
static int test_inline_coordinate_op_parsing()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText(
        "\\draw [fill=blue,blue,opacity=0.3] (2,1) coordinate (test) circle [radius=2mm];\n"
        "\\draw (0,0) -- (1,1) coordinate[pos=0.5] (mid) -- (2,0);\n"
        "\\path (5,5) coordinate (corner);\n");
    TikzDocumentState state;
    state.reparse(&doc);
    const auto &coords = state.userCoordinates();
    if (!coords.contains("test")) { fprintf(stderr, "FAIL: DCS-CO1 - inline 'coordinate (test)' missing\n"); failed++; }
    if (!coords.contains("mid")) { fprintf(stderr, "FAIL: DCS-CO2 - inline 'coordinate[pos=..] (mid)' missing\n"); failed++; }
    if (!coords.contains("corner")) { fprintf(stderr, "FAIL: DCS-CO3 - inline 'coordinate (corner)' missing\n"); failed++; }
    // Must also be treated as a node target for highlight/anchoring.
    if (!state.userNodes().contains("test")) { fprintf(stderr, "FAIL: DCS-CO4 - 'test' not registered as node\n"); failed++; }
    if (failed == 0) fprintf(stderr, "PASS: inline coordinate path-op parsing\n");
    return failed;
}

// The LaTeX template a snippet compiles against contributes packages and
// libraries too (e.g. default_circuit loads circuitikz). Regression: these
// were never fed into activeLibs, so circuitikz completions stayed off for
// circuit snippets whose metadata didn't repeat the package.
static int test_template_content_activates_libs()
{
    int failed = 0;

    QTextDocument doc;
    doc.setPlainText("\\begin{tikzpicture}\n\\draw (0,0) -- (1,1);\n\\end{tikzpicture}");

    // Real default_circuit template shape (options + PreviewEnvironment).
    const QString circuitTemplate = QStringLiteral(
        "\\documentclass[tikz, border=5pt]{standalone}\n"
        "\\usepackage{tikz}\n"
        "\\usepackage{xcolor}\n"
        "\\usepackage[europeanvoltages,betterproportions]{circuitikz}\n"
        "\\usepackage[active,tightpage]{preview}\n"
        "\\PreviewEnvironment{circuitikz}\n"
        "\\begin{document}\n"
        "%%% TIKZ_CODE_HERE %%%\n"
        "\\end{document}\n");

    TikzDocumentState state;
    state.setTemplateContent(circuitTemplate);
    state.reparse(&doc);
    if (!state.activeLibs().contains("circuitikz")) {
        fprintf(stderr, "FAIL: DCS-TP1 - template \\usepackage{circuitikz} should activate circuitikz\n");
        failed++;
    }

    // Template \usetikzlibrary lines are merged as-is.
    TikzDocumentState state2;
    state2.setTemplateContent(QStringLiteral(
        "\\usepackage{tikz}\n"
        "\\usetikzlibrary{calc, decorations.pathmorphing}\n"));
    state2.reparse(&doc);
    if (!state2.activeLibs().contains("calc")) {
        fprintf(stderr, "FAIL: DCS-TP2 - template \\usetikzlibrary{calc} missing\n");
        failed++;
    }
    if (!state2.activeLibs().contains("decorations.pathmorphing")) {
        fprintf(stderr, "FAIL: DCS-TP3 - template \\usetikzlibrary{decorations.pathmorphing} missing\n");
        failed++;
    }

    // Commented-out lines must not activate anything.
    TikzDocumentState state3;
    state3.setTemplateContent(QStringLiteral(
        "% \\usepackage{circuitikz}\n"
        "  % \\usetikzlibrary{calc}\n"
        "\\usepackage{tikz} % \\usepackage{chemfig}\n"));
    state3.reparse(&doc);
    if (state3.activeLibs().contains("circuitikz")) {
        fprintf(stderr, "FAIL: DCS-TP4 - commented \\usepackage must be ignored\n");
        failed++;
    }
    if (state3.activeLibs().contains("calc")) {
        fprintf(stderr, "FAIL: DCS-TP5 - commented \\usetikzlibrary must be ignored\n");
        failed++;
    }
    if (state3.activeLibs().contains("chemfig")) {
        fprintf(stderr, "FAIL: DCS-TP6 - inline-commented \\usepackage must be ignored\n");
        failed++;
    }

    // Template provides merge with snippet metadata (union).
    TikzDocumentState state4;
    state4.setTemplateContent(circuitTemplate);
    state4.setSnippetLibraries({"arrows.meta"});
    state4.setSnippetPackages({"pgfplots"});
    state4.reparse(&doc);
    if (!state4.activeLibs().contains("circuitikz")
        || !state4.activeLibs().contains("arrows.meta")
        || !state4.activeLibs().contains("pgfplots")) {
        fprintf(stderr, "FAIL: DCS-TP7 - template + metadata must merge\n");
        failed++;
    }

    // Switching to a template without the package deactivates it again.
    state4.setTemplateContent(QString());
    state4.reparse(&doc);
    if (state4.activeLibs().contains("circuitikz")) {
        fprintf(stderr, "FAIL: DCS-TP8 - cleared template must drop its libs\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: template content activates libs\n");
    return failed;
}

// Unified package→library mapping: metadata entries must behave like
// \usepackage parsing, including tkz-euclide (previously missing in the
// metadata path) and names carrying an option suffix.
static int test_metadata_package_implications()
{
    int failed = 0;
    QTextDocument doc;
    doc.setPlainText("\\begin{tikzpicture}\n\\end{tikzpicture}");

    TikzDocumentState state;
    state.setSnippetPackages({"tkz-euclide"});
    state.reparse(&doc);
    if (!state.activeLibs().contains("tkz-euclide")) {
        fprintf(stderr, "FAIL: DCS-MP1 - metadata 'tkz-euclide' should activate tkz-euclide\n");
        failed++;
    }

    TikzDocumentState state2;
    state2.setSnippetPackages({"circuitikz[europeanvoltages]", "TIKZ-CD"});
    state2.reparse(&doc);
    if (!state2.activeLibs().contains("circuitikz")) {
        fprintf(stderr, "FAIL: DCS-MP2 - option suffix must not break the mapping\n");
        failed++;
    }
    if (!state2.activeLibs().contains("cd")) {
        fprintf(stderr, "FAIL: DCS-MP3 - mapping must be case-insensitive\n");
        failed++;
    }

    if (failed == 0) fprintf(stderr, "PASS: metadata package implications\n");
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
    failed += test_foreach_spaces_around_slash();
    failed += test_foreach_multiple_vars();
    failed += test_foreach_optional_bracket();
    failed += test_color_parsing();
    failed += test_user_command_parsing();
    failed += test_current_env_name();
    failed += test_snippet_libraries();
    failed += test_circuitikz_scope();
    failed += test_style_with_spaces();
    failed += test_pic_name_detection();
    failed += test_usepackage_activates_libs();
    failed += test_node_name_in_options();
    failed += test_node_both_name_styles();
    failed += test_name_path_parsing();
    failed += test_by_coordinate_parsing();
    failed += test_inline_coordinate_op_parsing();
    failed += test_template_content_activates_libs();
    failed += test_metadata_package_implications();
    if (failed > 0) { fprintf(stderr, "\n%d test(s) failed!\n", failed); return 1; }
    fprintf(stderr, "\nAll TikzDocumentState tests passed!\n");
    return 0;
}
