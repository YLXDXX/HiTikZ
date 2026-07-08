#include "tikz_keywords.h"

namespace TikzKeywords {

using C = Category;
using Vec = QVector<TikzKeyword>;

static void addBuiltin(Vec &db, const char *name, Category cat,
                       std::initializer_list<const char *> envs = {},
                       std::initializer_list<const char *> cmds = {},
                       std::initializer_list<const char *> vals = {},
                       std::initializer_list<const char *> libs = {},
                       const char *doc = nullptr)
{
    TikzKeyword kw;
    kw.name = QString::fromUtf8(name);
    kw.category = cat;
    for (auto &e : envs) kw.environments.insert(QString::fromUtf8(e));
    for (auto &c : cmds) kw.commands.insert(QString::fromUtf8(c));
    for (auto &v : vals) kw.valueHints.append(QString::fromUtf8(v));
    for (auto &l : libs) kw.requiredLibs.insert(QString::fromUtf8(l));
    kw.doc = doc ? QString::fromUtf8(doc) : QString();
    db.append(kw);
}

#define E(...) __VA_ARGS__

static void addBipolePrefixes(Vec &db, const char *bipole,
                              std::initializer_list<const char *> envs)
{
    static const char *prefixes[] = {
        "p","v","i","ld","s","q","o","vq","iq","qq", nullptr
    };
    for (int p = 0; prefixes[p]; p++) {
        QByteArray name = QByteArray(prefixes[p]) + bipole;
        addBuiltin(db, name.constData(), C::Command, envs,
                   {"draw","path","to"}, {}, {}, nullptr);
    }
}

static void addCtikzShapes(Vec &db, std::initializer_list<const char *> names,
                           std::initializer_list<const char *> envs)
{
    for (auto &n : names) {
        QByteArray shapeName = QByteArray(n) + "shape";
        addBuiltin(db, shapeName.constData(), C::Shape, envs, {"node"});
    }
}

static void addCtikzShape(Vec &db, const char *shapeName,
                          std::initializer_list<const char *> envs)
{
    addBuiltin(db, shapeName, C::Shape, envs, {"node"});
}

void TikzKeywordDB::initBuiltins()
{
    Vec &db = m_builtin;

    // ── Colors ──
    const char *allColors[] = {"red","green","blue","cyan","magenta","yellow","black","white",
        "gray","darkgray","lightgray","brown","orange","purple","violet","pink","olive","teal","lime",
        "darkblue","darkgreen","darkred","darkcyan","darkmagenta","darkyellow","darkorange",
        "darkpurple","darkviolet","lightblue","lightgreen","lightred","lightcyan","lightmagenta",
        "lightyellow","lightorange","lightpink","copper","gold","silver","bronze",nullptr};
    for (int i = 0; allColors[i]; i++)
        addBuiltin(db, allColors[i], C::Color, {"tikzpicture","scope","axis","circuitikz"},
                   {"draw","path","fill","filldraw","shade","shadedraw","node"});

    // ── Line widths ──
    addBuiltin(db, "ultra thin",  C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "very thin",   C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "thin",        C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "semithick",   C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "thick",       C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "very thick",  C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "ultra thick", C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});

    // ── Line types ──
    addBuiltin(db, "solid",                  C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "dashed",                 C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "dotted",                 C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "dash dot",               C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "dash dot dot",           C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "loosely dashed",         C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "densely dashed",         C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "loosely dotted",         C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "densely dotted",         C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "loosely dash dot",       C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "densely dash dot",       C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "loosely dash dot dot",   C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});
    addBuiltin(db, "densely dash dot dot",   C::LineType, {"tikzpicture","scope","axis"}, {"draw","path","filldraw"});

    // ── Arrow specs (used as direct bracket keys or after >) ──
    addBuiltin(db, "->",  C::Arrow, {}, {"draw","path"});
    addBuiltin(db, "<-",  C::Arrow, {}, {"draw","path"});
    addBuiltin(db, "<->", C::Arrow, {}, {"draw","path"});

    // ── Arrow tip names ──
    const char *arrows[] = {"stealth","stealth'","latex","latex reversed","to","to reversed",
        "angle 90","angle 60","angle 45","triangle 90","triangle 60","triangle 45",
        "open triangle 90","open triangle 60","open triangle 45",
        "Circle","Diamond","Square","Bar","Bracket","Parenthesis",
        "Round Cap","Butt Cap","Triangle Cap","Fast Round","Fast Triangle",
        "Hooks","Implies","Straight Barb","Arc Barb","Classical TikZ Rightarrow",
        "Computer Modern Rightarrow","Stealth","Latex","To",nullptr};
    for (int i = 0; arrows[i]; i++)
        addBuiltin(db, arrows[i], C::Arrow, {}, {"draw","path"});

    // ── Shapes ──
    addBuiltin(db, "rectangle",               C::Shape, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "circle",                  C::Shape, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "ellipse",                 C::Shape, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "diamond",                 C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "coordinate",              C::Shape, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "circle split",            C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});
    addBuiltin(db, "rectangle split",         C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});
    addBuiltin(db, "rounded rectangle",       C::Shape, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "regular polygon",         C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "isosceles triangle",      C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "trapezium",               C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "semicircle",              C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "circular sector",         C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "cylinder",                C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "single arrow",            C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.arrows"});
    addBuiltin(db, "double arrow",            C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.arrows"});
    addBuiltin(db, "cloud",                   C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.symbols"});
    addBuiltin(db, "star",                    C::Shape, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "polygon",                 C::Shape, {"tikzpicture","scope"}, {"node"});

    // ── Patterns ──
    addBuiltin(db, "horizontal lines",  C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "vertical lines",    C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "north east lines",  C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "north west lines",  C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "crosshatch",        C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "crosshatch dots",   C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "bricks",            C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "checkerboard",      C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "grid",              C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "dots",              C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "fivepointed stars", C::Pattern, {}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "sixpointed stars",  C::Pattern, {}, {"draw","path","fill","filldraw"});

    // ── Decorations ──
    addBuiltin(db, "snake",    C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "coil",     C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "saw",      C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "zigzag",   C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "bumps",    C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "random steps", C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "lineto",   C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "curveto",  C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "straight zigzag", C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "ticks",    C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "crosses",  C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "brace",        C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "brace mirrored", C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "curly brace",  C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "triangle",     C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "angle",        C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "trapezium brace", C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "markings", C::Decoration, {}, {"draw","path"}, {}, {"decorations.markings"});
    addBuiltin(db, "Koch snowflake",  C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Koch curve",      C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Koch curve type 2", C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Hilbert curve",   C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Sierpinski triangle", C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Cantor set",      C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Peano curve",     C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Moore curve",     C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Levy curve",      C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "text along path", C::Decoration, {}, {"draw","path"}, {}, {"decorations.text"});
    addBuiltin(db, "footprints",      C::Decoration, {}, {"draw","path"}, {}, {"decorations.footprints"});
    addBuiltin(db, "stars",           C::Decoration, {}, {"draw","path"}, {}, {"decorations.shapes"});
    addBuiltin(db, "triangles",       C::Decoration, {}, {"draw","path"}, {}, {"decorations.shapes"});
    addBuiltin(db, "shape backgrounds", C::Decoration, {}, {"draw","path"}, {}, {"decorations.shapes"});

    // ── Anchors ──
    const char *anchors[] = {
        "north","north east","north west","south","south east","south west",
        "east","west","center","base","base east","base west",
        "mid","mid east","mid west","text","text split",
        "left","right","top","bottom",
        "apex","corner 1","corner 2","tail",
        "input","output","input 1","input 2","output 1","output 2",
        "angle","150","120","90","60","30","0","210","240","270","300","330",
        "wiper","W",
        "cathode","anode","gate","G",
        "in","in 1","in 2","out","out 1","out 2",
        "tap","tap down","tap up","v+","v-","tip",
        "B","C","E","S","D",
        "collector","emitter","base",
        "source","drain","bulk","substrate",
        "primary","secondary",
        "primary left","primary right",
        "secondary left","secondary right",
        "+","-",
        "A","B","C","Y","O",
        "text split one","text split two",
        "part one","part two","part three","part four",
        "lower","upper",
        nullptr};
    for (int i = 0; anchors[i]; i++)
        addBuiltin(db, anchors[i], C::Anchor, {}, {"node"});

    // ── Key handlers ──
    addBuiltin(db, "style",             C::Handler);
    addBuiltin(db, "default",           C::Handler);
    addBuiltin(db, "code",              C::Handler);
    addBuiltin(db, "append style",      C::Handler);
    addBuiltin(db, "prefix style",      C::Handler);
    addBuiltin(db, "initial",           C::Handler);
    addBuiltin(db, "add",               C::Handler);
    addBuiltin(db, "store in",          C::Handler);
    addBuiltin(db, "estore in",         C::Handler);
    addBuiltin(db, "value required",    C::Handler);
    addBuiltin(db, "value forbidden",   C::Handler);
    addBuiltin(db, "try",               C::Handler);
    addBuiltin(db, "retry",             C::Handler);
    addBuiltin(db, "search also",       C::Handler);
    addBuiltin(db, "cd",                C::Handler);
    addBuiltin(db, "handler",           C::Handler);

    // ── PGF key paths ──
    addBuiltin(db, "/tikz",                     C::PGFKeyPath);
    addBuiltin(db, "/pgf",                      C::PGFKeyPath);
    addBuiltin(db, "/pgfplots",                 C::PGFKeyPath);
    addBuiltin(db, "/graph",                    C::PGFKeyPath);
    addBuiltin(db, "/tikz/every picture",       C::PGFKeyPath);
    addBuiltin(db, "/tikz/every node",          C::PGFKeyPath);
    addBuiltin(db, "/tikz/every path",          C::PGFKeyPath);
    addBuiltin(db, "/tikz/every edge",          C::PGFKeyPath);
    addBuiltin(db, "/tikz/every label",         C::PGFKeyPath);
    addBuiltin(db, "/tikz/every pin",           C::PGFKeyPath);
    addBuiltin(db, "/tikz/every graph",         C::PGFKeyPath);
    addBuiltin(db, "/tikz/every plot",          C::PGFKeyPath);
    addBuiltin(db, "/pgf/number format",        C::PGFKeyPath);
    addBuiltin(db, "/pgfplots/legend",          C::PGFKeyPath);
    addBuiltin(db, "/circuitikz",               C::PGFKeyPath);
    // CircuiTikZ specific key paths
    addBuiltin(db, "/tikz/circuitikz",          C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles",             C::PGFKeyPath);
    addBuiltin(db, "/tikz/tripoles",            C::PGFKeyPath);
    addBuiltin(db, "/tikz/quadpoles",           C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/generic",     C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/resistor",    C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/capacitor",   C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/inductor",    C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/diode",       C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/source",      C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/meter",       C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/switch",      C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/mechanical",  C::PGFKeyPath);
    addBuiltin(db, "/tikz/bipoles/transistor",  C::PGFKeyPath);
    addBuiltin(db, "/tikz/tripoles/op amp",     C::PGFKeyPath);
    addBuiltin(db, "/tikz/tripoles/bjt",        C::PGFKeyPath);
    addBuiltin(db, "/tikz/tripoles/igbt",       C::PGFKeyPath);
    addBuiltin(db, "/tikz/tripoles/mos",        C::PGFKeyPath);
    addBuiltin(db, "/tikz/tripoles/jfet",       C::PGFKeyPath);
    addBuiltin(db, "/tikz/tripoles/tube",       C::PGFKeyPath);
    addBuiltin(db, "/tikz/quadpoles/transformer", C::PGFKeyPath);
    addBuiltin(db, "/tikz/quadpoles/fourport",  C::PGFKeyPath);
    addBuiltin(db, "/tikz/quadpoles/gyrator",   C::PGFKeyPath);
    addBuiltin(db, "/tikz/quadpoles/coupling",  C::PGFKeyPath);

    // ── Libraries ──
    const char *libs[] = {
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
        "decorations.shapes",
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
        "arrows.meta","trees",
        "topaths","graphs.standard","babel",
        "cd","circuitikz","tkz-euclide",
        "shapes.gates.logic","shapes.gates.logic.IEC","shapes.gates.logic.US",
        nullptr};
    for (int i = 0; libs[i]; i++)
        addBuiltin(db, libs[i], C::Library);

    // ── Environments ──
    const char *envs[] = {
        "tikzpicture","scope","pgfonlayer",
        "axis","semilogxaxis","semilogyaxis","loglogaxis",
        "polaraxis","smithchart","ternaryaxis","groupplot",
        "datavisualization","tikzcd","forest","mindmap","feynman","circuitikz",
        "filecontents","filecontents*","comment",
        "document","center","flushleft","flushright",
        "itemize","enumerate","description",
        "figure","table","minipage",
        "align","align*","equation","equation*",
        "displaymath","math",
        "abstract","proof","theorem","lemma","corollary","definition",
        "array",
        "matrix","pmatrix","bmatrix","Bmatrix","vmatrix","Vmatrix",
        "cases","split","gathered","aligned","alignedat",
        "gather","gather*","multline","multline*",
        "tabularx","longtable","tabular",
        "wrapfigure","tcolorbox","overpic",
        "quote","quotation","verse","appendix","frame",nullptr};
    for (int i = 0; envs[i]; i++)
        addBuiltin(db, envs[i], C::Environment);

    // ── Commands ── (LaTeX/TikZ/PGF commands)
    // ── CircuiTikZ path components (as Options, not Commands) ──
    // These are used in to[...] or node[...] as keys, not as \ backslash commands.
    // Filtering is done via requiredLibs = {"circuitikz"}.
    const char *ctikzComponents[] = {
        "R","V","I","C","L","D","short","open",
        "resistor","vresistor","capacitor","vcapacitor","inductor",
        "battery","battery1","battery2",
        "diode","Zdiode","Sdiode","Tdiode","led","pD",
        "varistor","thermistor","photoresistor","memristor",
        "fuse","lamp","bulb",
        "spring","mass",
        "switch","closingswitch","openingswitch",
        "ammeter","voltmeter","ohmmeter",
        "isource","vsource","dcvsource","dcisource",
        "acsource","vsourceAC","isourceAC",
        "generic","tgeneric","ageneric","emptygeneric","fullgeneric",
        "potentiometer",
        "capacitive","polarcapacitor","ecapacitor",
        "transformer","coreless transformer","transducer",
        "elmech","elco","pvarcapacitor","vvarcapacitor",
        "cute_inductor","american_inductor","coils","vcoils","cute_coil","american_coil",
        "vsourcesin","isourcesin","vsourceDC","isourceDC","solarcell",
        "zdiode","schottky","photodiode","varactor","tunneldiode",
        "thyristor","triac","diac",
        "nigfet","pigfet","nigfete","pigfete","nmosfete","pmosfete",
        "njfet","pjfet","nchenh","nchdep","pchdep","nch","pch",
        "nigmfet","pigmfet","Lnigfet","Lpigfet",
        "rmeter","rmeterwa","smeter","qiprobe","qvprobe","qprobe",
        "iloop2","iloop","viscoe",
        "cspst","ospt","cswitch","oswitch","pushbutton","nopb","ncpb",
        "motor","motor2","gear",
        "twoport","fourport","transformercore","gyrator","ctline","delayline","tline",
        "ocirc","fcirc","ccirc","ocorner","icorner",
        "ground","rground","cground","sground","tground","noground",
        "node ground","crossover",
        "pnp","npn","pnp alt","npn alt",
        "pmos","nmos","pmos alt","nmos alt",
        "pigfete","nigfete","pigfetd","nigfetd",
        "pfet","nfet",
        "op amp","tlmop amp","put",
        "tube","tube triode","tube tetrode","tube pentode","barrier",
        "american and gate","american or gate","american not gate",
        "american nand gate","american nor gate","american xor gate","american xnor gate",
        "american buffer gate","american inverter gate",
        "and port","or port","not port",
        "nand port","nor port","xor port","xnor port",
        "buffer port","inverter port",
        nullptr
    };
    for (int i = 0; ctikzComponents[i]; i++) {
        addBuiltin(db, ctikzComponents[i], C::Option, {}, {"draw","path","to"}, {}, {"circuitikz"});
    }

    // ═══════════════════════════════════════════════════════════════
    //  tkz-euclide v5 commands
    //  Activated by \usepackage{tkz-euclide}
    // ═══════════════════════════════════════════════════════════════
    auto tkeEnvs = {"tikzpicture","scope"};

    addBuiltin(db, "tkzDefPoint",            C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDefPoints",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDefMidPoint",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDefBarycentricPoint", C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDefShiftPoint",       C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDefPointBy",          C::Command, tkeEnvs, {},
               {"translation","rotation","homothety","reflection","symmetry","projection","inversion"},
               {"tkz-euclide"});
    addBuiltin(db, "tkzDefPointWith",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzGetPoint",            C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzGetPoints",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzGetLength",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzGetAngle",            C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDefLine",             C::Command, tkeEnvs, {},
               {"parallel","perpendicular","mediator","bisector"},
               {"tkz-euclide"});
    addBuiltin(db, "tkzDefCircle",           C::Command, tkeEnvs, {},
               {"through","R","diameter","in","circum","ex","apollonius"},
               {"tkz-euclide"});

    addBuiltin(db, "tkzDefSquare",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDefRegPolygon",       C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDefTriangle",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDefTriangleCenter",   C::Command, tkeEnvs, {},
               {"centroid","in","circum","ortho","euler","nine","ex"},
               {"tkz-euclide"});
    addBuiltin(db, "tkzDefPolygonCenter",    C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawPoint",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawPoints",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawLine",            C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawLines",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawSegment",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawSegments",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawHalfLine",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawHalfLines",       C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawPolygon",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawPolygons",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillPolygon",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillPolygons",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawSquare",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawRegPolygon",      C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillSquare",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillRegPolygon",      C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawTriangle",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillTriangle",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawCircle",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawCircles",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillCircle",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillCircles",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawArc",             C::Command, tkeEnvs, {},
               {"R","delta"}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillArc",             C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawAngle",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillAngle",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzFillAngles",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzLabelAngle",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzLabelAngles",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzMarkAngle",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzMarkAngles",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzMarkRightAngle",      C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzMarkRightAngles",     C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzLabelPoint",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzLabelPoints",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzLabelSegment",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzLabelSegments",       C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzLabelLine",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzLabelCircle",         C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzInterLL",             C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzInterLC",             C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzInterCC",             C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzGetFirstPoint",       C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzGetSecondPoint",      C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzTangent",             C::Command, tkeEnvs, {},
               {"from","at","external"}, {"tkz-euclide"});

    addBuiltin(db, "tkzDrawBisector",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawMedian",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawAltitude",        C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzDrawEulerLine",       C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzClipCircle",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzClipLine",            C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    addBuiltin(db, "tkzSetUpPoint",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzSetUpLine",           C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});
    addBuiltin(db, "tkzSetUpLabel",          C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    // Bipole prefix combinations (10 prefixes × 9 types = 90 options)
    {
        static const char *prefixes[] = {
            "p","v","i","ld","s","q","o","vq","iq","qq", nullptr
        };
        static const char *bipoles[] = {
            "R","L","C","D","V","I","Q","Ty","Tr", nullptr
        };
        for (int p = 0; prefixes[p]; p++)
            for (int b = 0; bipoles[b]; b++) {
                QByteArray sn = QByteArray(prefixes[p]) + bipoles[b];
                addBuiltin(db, sn.constData(), C::Option, {}, {"draw","path","to"}, {}, {"circuitikz"});
            }
    }

    const char *cmds[] = {
        "addplot","addplot3","addlegendentry","addlegendimage",
        "anchor","arc","arrow",
        "ball","begin","bend",
        "caption","circle","clip","closedcycle","coordinate",
        "coordinates","cos","cycle",
        "dataset","datavisualization",
        "definecolor","def","draw","drawplot",
        "edef","edge","else","end","exp","expandafter",
        "fill","filldraw","flat","foreach","forest",
        "getref","gdef","global","graph",
        "hbox","hfil","hfill","hline","hrule","hskip","hspace",
        "if","ifdim","ifnum","ifodd","ifx",
        "input","item",
        "label","let","line","lineto","ln",
        "matrix","max","min","moveleft","moveright",
        "newcommand","newcount","newdimen","newif","newwrite",
        "node",
        "par","parbox","path","pattern","pgfkeys","pgfmathsetmacro",
        "pgfplotstableread","pgfplotstabletypeset",
        "pic","pin","plot","protected",
        "relax","renewcommand","repeat",
        "scope","setlength","shade","shadedraw",
        "sin","sqrt",
        "tabular","text","tikzset","tikzstyle","to",
        "usebox","usepackage","usetikzlibrary",
        "vbox","vertex","vfil","vfill","vrule","vskip","vspace",
        "xdef",
        "tdplotsetmaincoords","tdplotsetrotatedcoords",
        "tdplotsetrotatedcoordsorigin","tdplotresetrotatedcoordsorigin",
        "tdplotsetcoord","tdplotsetthetaplanecoords",
        "tdplotsetrotatedthetaplanecoords",
        "tdplotsetpolarplotrange","tdplotresetpolarplotrange",
        "tdplotdrawarc","tdplotdrawpolytopearc",
        "tdplotsphericalsurfaceplot","tdplotshowargcolorguide",
        "tdplotdefinepoints","tdplotcalctransformmainrot",
        "tdplotcalctransformrotmain","tdplotcalctransformmainscreen",
        "tdplottransformmainrot","tdplottransformrotmain",
        "tdplottransformmainscreen","tdplotgetpolarcoords",
        "tdplotcrossprod","tdplotsinandcos","tdplotmult","tdplotdiv",
        "pgfmathtruncatemacro","pgfmathparse","pgfmathresult",
        "pgfmathprintnumber","pgfmathrandominteger","pgfmathsetseed",
        "pgfmathsetmacroglobal","pgfkeysvalueof","pgfkeysgetvalue",
        "pgfkeyssetvalue","pgfplotsset",
        "pgfdeclarelayer","pgfsetlayers","pgfonlayer","endpgfonlayer",
        "pgfdeclareshape","pgfdeclareplotmark","pgfdeclarepattern",
        "pgfdeclaredecoration","pgfdeclarehorizontalshading",
        "pgfdeclareverticalshading",
        "pgfsetlinewidth","pgfsetcolor","pgfsetarrow","pgfsetdash",
        "pgfsetbuttcap","pgfsetroundcap","pgfsetrectcap",
        "pgfsetroundjoin","pgfsetbeveljoin","pgfsetmiterjoin",
        "pgftext","pgfnode","pgfcoordinate","pgfpathmoveto",
        "pgfpathlineto","pgfpathcurveto","pgfpathcircle",
        "pgfpathellipse","pgfpathrectangle","pgfpathclose",
        "pgfusepath","pgfuseplotmark","pgfpoint","pgfpointanchor",
        "pgftransformshift","pgftransformrotate","pgftransformscale",
        "pgftransformxscale","pgftransformyscale",
        "pgftransformxslant","pgftransformyslant",
        "pgftransformcm","pgftransformreset",
        "useasboundingbox","tikztonodes","tikzlastnode","tikzmark","legend",
        // Greek
        "alpha","beta","gamma","delta","epsilon","varepsilon",
        "zeta","eta","theta","vartheta","iota","kappa",
        "lambda","mu","nu","xi","pi","varpi","rho","varrho",
        "sigma","varsigma","tau","upsilon","phi","varphi",
        "chi","psi","omega",
        "Gamma","Delta","Theta","Lambda","Xi","Pi","Sigma",
        "Upsilon","Phi","Psi","Omega",
        // Math operators
        "arccos","arcsin","arctan","arg","cot","coth","csc",
        "deg","det","dim","hom","ker","lg","lim","liminf","limsup","sup","inf",
        "gcd","Pr","sec","sinh","cosh","tanh","log",
        // Delimiters
        "left","right","bigl","Bigl","biggl","Biggl",
        "bigr","Bigr","biggr","Biggr",
        "langle","rangle","lceil","rceil","lfloor","rfloor",
        "lvert","rvert","lVert","rVert",
        // Accents
        "hat","check","tilde","acute","grave","dot","ddot",
        "breve","bar","vec","widehat","widetilde",
        "overline","underline","overrightarrow","overleftarrow",
        "overbrace","underbrace",
        // Math fonts
        "mathbb","mathbf","mathrm","mathsf","mathtt",
        "mathit","mathcal","mathscr","mathfrak",
        "text","textbf","textit","texttt","textsf","textsc",
        // Math
        "frac","dfrac","tfrac","binom","dbinom","tbinom",
        "int","iint","iiint","oint",
        "sum","prod","coprod",
        "bigcup","bigcap","bigvee","bigwedge",
        "bigoplus","bigotimes","bigodot","biguplus","bigsqcup",
        "partial","nabla","forall","exists","nexists","emptyset","varnothing",
        "infty","aleph","hbar","imath","jmath","ell","wp","Re","Im",
        "angle","measuredangle",
        "to","longrightarrow","longleftarrow","longleftrightarrow",
        "rightarrow","leftarrow","leftrightarrow",
        "mapsto","longmapsto",
        "Rightarrow","Leftarrow","Leftrightarrow",
        "Longrightarrow","Longleftarrow","Longleftrightarrow",
        "approx","sim","simeq","cong","equiv",
        "propto","neq","pm","mp",
        "times","div","cdot","ast","star",
        "otimes","oplus","ominus","odot","oslash",
        "cap","cup","setminus","subset","supset",
        "in","ni","notin",
        "leq","geq","ll","gg","parallel","perp","lll","ggg",
        "subseteq","supseteq","subsetneq","supsetneq",
        "smile","frown","bowtie","ldots","cdots","vdots","ddots",
        "xleftarrow","xrightarrow",
        "displaystyle","textstyle","scriptstyle","scriptscriptstyle",
        // Spacing
        "quad","qquad","hspace","vspace",
        // Boxes
        "mbox","makebox","framebox","noindent","centering","raggedright","raggedleft",
        // LaTeX
        "title","author","date","maketitle",
        "chapter","section","subsection","subsubsection",
        "emph","url","href","includegraphics",
        "color","textcolor","colorbox","fcolorbox",
        // CircuitikZ
        "resistor","vresistor","potentiometer",
        "thermistor","thermistorptc","thermistorntc",
        "varistor","mov","memristor","photoresistor",
        "generic","ageneric","tgeneric",
        "short","open",
        "capacitor","ecapacitor","ccapacitor","vcapacitor",
        "ferrocap","piezoelectric","cpe",
        "inductor",
        "battery","battery1","battery2",
        "lamp","bulb","fuse","relais","squid","barrier",
        "thermocouple","loudspeaker","mic","buzzer",
        "ammeter","voltmeter","ohmmeter","oscope",
        nullptr};
    for (int i = 0; cmds[i]; i++)
        addBuiltin(db, cmds[i], C::Command);

    // ── Math functions ──
    const char *mathFns[] = {
        "abs","acos","add","angle","asin","atan","atan2",
        "ceil","cit","clip","cm","cos","cosec","cosh",
        "cot","deg","depth","dim","divide","dotproduct",
        "e","exp","factorial","floor","format",
        "frac","gcd","height",
        "in","int","intersection","isodd",
        "kil","length","ln","log10","log2",
        "max","min","mod","multiply",
        "not","note","pi","pow","power",
        "pt","rad","rand","random","rdiv","real","record","res",
        "round","scalar","sec","sign","sin","sinh",
        "sqrt","subtract","sum",
        "tan","tanh","true","turn",
        "value","vecilen","width",nullptr};
    for (int i = 0; mathFns[i]; i++)
        addBuiltin(db, mathFns[i], C::MathFunction);

    // ═══════════════════════════════════════════════════
    // ── GENERAL OPTIONS (universal tikz keys) ──
    // ═══════════════════════════════════════════════════

    // Positioning
    addBuiltin(db, "above",        C::Option, {}, {"node","pic"});
    addBuiltin(db, "above left",   C::Option, {}, {"node","pic"});
    addBuiltin(db, "above right",  C::Option, {}, {"node","pic"});
    addBuiltin(db, "below",        C::Option, {}, {"node","pic"});
    addBuiltin(db, "below left",   C::Option, {}, {"node","pic"});
    addBuiltin(db, "below right",  C::Option, {}, {"node","pic"});
    addBuiltin(db, "left",         C::Option, {}, {"node","pic"});
    addBuiltin(db, "right",        C::Option, {}, {"node","pic"});
    addBuiltin(db, "left=of",      C::Option, {"tikzpicture","scope"}, {"node","pic"}, {}, {"positioning"});
    addBuiltin(db, "right=of",     C::Option, {"tikzpicture","scope"}, {"node","pic"}, {}, {"positioning"});
    addBuiltin(db, "above=of",     C::Option, {"tikzpicture","scope"}, {"node","pic"}, {}, {"positioning"});
    addBuiltin(db, "below=of",     C::Option, {"tikzpicture","scope"}, {"node","pic"}, {}, {"positioning"});
    addBuiltin(db, "on grid",      C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"positioning"});
    addBuiltin(db, "node distance",C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","2.5cm","3cm","4cm"}, {"positioning"});

    // Common transforms
    addBuiltin(db, "rotate",          C::Option, {}, {},
               {"90","180","270","45","60","30","15"});
    addBuiltin(db, "rotate around",   C::Option, {});
    addBuiltin(db, "scale",           C::Option, {}, {},
               {"0.5","0.8","1.0","1.2","1.5","2.0","3.0"});
    addBuiltin(db, "scale around",    C::Option, {});
    addBuiltin(db, "xscale",          C::Option, {});
    addBuiltin(db, "yscale",          C::Option, {});
    addBuiltin(db, "xshift",          C::Option, {});
    addBuiltin(db, "yshift",          C::Option, {});
    addBuiltin(db, "xslant",          C::Option, {});
    addBuiltin(db, "yslant",          C::Option, {});
    addBuiltin(db, "transform canvas",C::Option, {});
    addBuiltin(db, "transform shape", C::Option, {});
    addBuiltin(db, "reset cm",        C::Option, {});
    addBuiltin(db, "shift",           C::Option, {});

    // Color / opacity
    addBuiltin(db, "color",           C::Option, {}, {"draw","path","fill","node"},
               {"red","green","blue","cyan","magenta","yellow","black","white","gray","orange"});
    addBuiltin(db, "draw",            C::Option, {}, {"draw","path","node","edge"},
               {"red","green","blue","cyan","magenta","yellow","black","white","gray","orange"});
    addBuiltin(db, "fill",            C::Option, {}, {"draw","path","node"},
               {"red","green","blue","cyan","magenta","yellow","black","white","gray","orange"});
    addBuiltin(db, "draw opacity",    C::Option, {}, {"draw","path"},
               {"0.3","0.5","0.7","1.0"});
    addBuiltin(db, "fill opacity",    C::Option, {}, {"draw","path","node"},
               {"0.3","0.5","0.7","1.0"});
    addBuiltin(db, "text opacity",    C::Option, {}, {"node"},
               {"0.5","0.7","1.0"});
    addBuiltin(db, "opacity",         C::Option, {}, {"draw","path","node"},
               {"0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"});
    addBuiltin(db, "left color",      C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "right color",     C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "middle color",    C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "top color",       C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "bottom color",    C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "inner color",     C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "outer color",     C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "ball color",      C::Option, {}, {"node"});
    addBuiltin(db, "pattern color",   C::Option, {}, {"draw","path","fill","filldraw"});

    // Line / stroke
    addBuiltin(db, "line width",          C::Option, {}, {"draw","path","filldraw"},
               {"0.2pt","0.4pt","0.5pt","0.6pt","0.8pt","1pt","1.2pt","1.5pt","2pt","2.5pt","3pt","4pt","5pt","6pt","8pt","10pt"});
    addBuiltin(db, "line cap",            C::Option, {}, {"draw","path"},
               {"butt","round","rect"});
    addBuiltin(db, "line join",           C::Option, {}, {"draw","path"},
               {"round","bevel","miter"});
    addBuiltin(db, "miter limit",         C::Option, {}, {"draw","path"});
    addBuiltin(db, "double",              C::Option, {}, {"draw","path"});
    addBuiltin(db, "double distance",     C::Option, {}, {"draw","path"},
               {"1pt","1.5pt","2pt","3pt","4pt","5pt"});
    addBuiltin(db, "double equal sign distance", C::Option, {}, {"draw","path"});
    addBuiltin(db, "dash",                C::Option, {}, {"draw","path"});
    addBuiltin(db, "dash phase",          C::Option, {}, {"draw","path"});
    addBuiltin(db, "solid",               C::Option, {}, {"draw","path"});
    addBuiltin(db, "rounded corners",     C::Option, {}, {"draw","path","node"},
               {"1pt","2pt","3pt","4pt","5pt","8pt","10pt"});
    addBuiltin(db, "cap",                 C::Option, {}, {"draw","path"}, {"butt","round","rect"}, {});
    addBuiltin(db, "join",                C::Option, {}, {"draw","path"}, {"round","bevel","miter"}, {});
    addBuiltin(db, "shorten <",           C::Option, {}, {"draw","path"},
               {"1pt","2pt","3pt","5pt","8pt","10pt"});
    addBuiltin(db, "shorten >",           C::Option, {}, {"draw","path"},
               {"1pt","2pt","3pt","5pt","8pt","10pt"});

    // Arrows key
    addBuiltin(db, "arrow",            C::Option, {}, {"draw","path"},
               {"stealth","latex","to","angle 90","angle 60","triangle 90",
                "Triangle","Circle","Diamond","Square","Bar","Bracket",
                "Parenthesis","Round Cap","Butt Cap","Triangle Cap",
                "Fast Round","Fast Triangle","Straight Barb","Arc Barb",
                "Computer Modern Rightarrow","Implies","Hooks","Classical TikZ Rightarrow"});

    // Node-specific
    addBuiltin(db, "shape",            C::Option, {"tikzpicture","scope"}, {"node"},
               {"rectangle","circle","ellipse","diamond","regular polygon",
                "isosceles triangle","trapezium","semicircle","circular sector",
                "cylinder","single arrow","double arrow","cloud","star",
                "rectangle split","circle split","rounded rectangle"});
    addBuiltin(db, "anchor",           C::Option, {}, {"node"},
               {"north","south","east","west","north east","north west",
                "south east","south west","center","base","base east","base west",
                "mid","mid east","mid west","text"});
    addBuiltin(db, "label",            C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "pin",              C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "pin distance",     C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "pin edge",         C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "pin position",     C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "name",             C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "name prefix",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "name suffix",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "alias",            C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "execute at begin node", C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "execute at end node",   C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "late options",     C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "behind path",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "in front of path", C::Option, {"tikzpicture","scope"}, {"node"});

    // Node dimensions
    addBuiltin(db, "minimum width",    C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"});
    addBuiltin(db, "minimum height",   C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"});
    addBuiltin(db, "minimum size",     C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"});
    addBuiltin(db, "inner sep",        C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","1pt","2pt","3pt","4pt","5pt","6pt","8pt","10pt"});
    addBuiltin(db, "inner xsep",       C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","2pt","4pt","6pt","8pt","10pt"});
    addBuiltin(db, "inner ysep",       C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","2pt","4pt","6pt","8pt","10pt"});
    addBuiltin(db, "outer sep",        C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","1pt","2pt","3pt","4pt","5pt"});
    addBuiltin(db, "outer xsep",       C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "outer ysep",       C::Option, {"tikzpicture","scope"}, {"node"});

    // Text in nodes
    addBuiltin(db, "text",             C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text width",       C::Option, {"tikzpicture","scope"}, {"node"},
               {"2cm","3cm","4cm","5cm","6cm","8cm","10cm"});
    addBuiltin(db, "text height",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text depth",       C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text centered",    C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text ragged",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text badly ragged",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text badly centered",    C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "align",            C::Option, {"tikzpicture","scope"}, {"node"},
               {"left","center","right","justify","none"});
    addBuiltin(db, "font",             C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "fontsize",         C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "node contents",    C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "node distance",    C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","2.5cm","3cm","4cm"});

    // Path decorations
    addBuiltin(db, "decorate",        C::Option, {}, {"draw","path"}, {}, {"decorations.pathmorphing","decorations.pathreplacing","decorations.markings"});
    addBuiltin(db, "decoration",      C::Option, {}, {"draw","path"},
               {"snake","coil","saw","zigzag","bumps","brace","markings"},
               {"decorations.pathmorphing","decorations.pathreplacing","decorations.markings"});

    // Bends / curves
    addBuiltin(db, "bend left",        C::Option, {}, {"draw","path","edge"},
               {"90","60","45","30","15"});
    addBuiltin(db, "bend right",       C::Option, {}, {"draw","path","edge"},
               {"90","60","45","30","15"});
    addBuiltin(db, "bend angle",       C::Option, {}, {"draw","path","edge"},
               {"90","60","45","30","15"});
    addBuiltin(db, "bend at start",    C::Option, {}, {"draw","path","edge"});
    addBuiltin(db, "bend at end",      C::Option, {}, {"draw","path","edge"});
    addBuiltin(db, "bend pos",         C::Option, {}, {"draw","path","edge"});
    addBuiltin(db, "in",               C::Option, {}, {"draw","path"},
               {"0","30","45","60","90","120","135","150","180","-135","-120","-90","-60","-45","-30"});
    addBuiltin(db, "out",              C::Option, {}, {"draw","path"},
               {"0","30","45","60","90","120","135","150","180","-135","-120","-90","-60","-45","-30"});
    addBuiltin(db, "in looseness",     C::Option, {}, {"draw","path"});
    addBuiltin(db, "out looseness",    C::Option, {}, {"draw","path"});
    addBuiltin(db, "looseness",        C::Option, {}, {"draw","path"});
    addBuiltin(db, "auto",             C::Option, {}, {"draw","path","node","edge"});
    addBuiltin(db, "swap",             C::Option, {}, {"draw","path","node","edge"});
    addBuiltin(db, "sloped",           C::Option, {}, {"draw","path","node","edge"});

    // Path positioning
    addBuiltin(db, "pos",              C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "midway",           C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "near start",       C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "near end",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "very near start",  C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "very near end",    C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "at start",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "at end",           C::Option, {}, {"draw","path","node"});

    // Misc drawing
    addBuiltin(db, "smooth",           C::Option, {}, {"draw","path"});
    addBuiltin(db, "tension",          C::Option, {}, {"draw","path"});
    addBuiltin(db, "append after command", C::Option, {}, {"node","pic","path"});
    addBuiltin(db, "preaction",        C::Option, {}, {"draw","path"});
    addBuiltin(db, "postaction",       C::Option, {}, {"draw","path"});
    addBuiltin(db, "overlay",          C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "remember picture", C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "baseline",         C::Option, {"tikzpicture"});
    addBuiltin(db, "every picture",    C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every node",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every edge",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every loop",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every label",      C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every pin",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every to",         C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every scope",      C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "label position",   C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "drop shadow",      C::Option, {}, {"node"}, {}, {"shadows"});
    addBuiltin(db, "circular drop shadow", C::Option, {}, {"node"}, {}, {"shadows"});
    addBuiltin(db, "visible on",       C::Option, {}, {"node","draw"}, {}, {"overlay-beamer-styles"});
    addBuiltin(db, "no draw",          C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "no fill",          C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "clip",             C::Option, {"tikzpicture","scope"}, {"draw","path"});

    // Path construction keywords (also available as options for some contexts)
    addBuiltin(db, "controls",         C::Option, {}, {"draw","path"});
    addBuiltin(db, "parabola",         C::Option, {}, {"draw","path"});

    // Fill rules
    addBuiltin(db, "nonzero rule",     C::Option, {}, {"draw","path","fill"});
    addBuiltin(db, "even odd rule",    C::Option, {}, {"draw","path","fill"});

    // Arc
    addBuiltin(db, "start angle",      C::Option, {}, {"draw","path"},
               {"0","30","45","60","90","120","135","150","180","210","225","240","270","300","315","330"});
    addBuiltin(db, "end angle",        C::Option, {}, {"draw","path"},
               {"0","30","45","60","90","120","135","150","180","210","225","240","270","300","315","330","360"});
    addBuiltin(db, "delta angle",      C::Option, {}, {"draw","path"});

    // Ellipse / arc dimensions
    addBuiltin(db, "x radius",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "y radius",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "z radius",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "radius",           C::Option, {}, {"draw","path","node"},
               {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"});

    // Matrix
    addBuiltin(db, "matrix of nodes",       C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "matrix of math nodes",  C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "column sep",            C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "row sep",               C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "nodes",                 C::Option, {}, {"draw","path"}, {}, {"matrix"});
    addBuiltin(db, "delimiters",            C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "left delimiter",        C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "right delimiter",       C::Option, {}, {"node"}, {}, {"matrix"});

    // Angles library
    addBuiltin(db, "angle radius",     C::Option, {}, {},
               {"0.3cm","0.5cm","0.8cm","1cm","1.5cm","2cm","3cm"}, {"angles"});
    addBuiltin(db, "angle eccentricity", C::Option, {}, {},
               {"0.3","0.5","0.6","0.8","1.0","1.2","1.5"}, {"angles"});
    addBuiltin(db, "right angle",      C::Option, {}, {}, {}, {"angles"});
    addBuiltin(db, "pic text",         C::Option, {}, {}, {}, {"angles","quotes"});

    // Intersections library
    addBuiltin(db, "name path",         C::Option, {}, {"draw","path"}, {}, {"intersections"});
    addBuiltin(db, "name path global",  C::Option, {}, {"draw","path"}, {}, {"intersections"});
    addBuiltin(db, "name intersections",C::Option, {}, {"draw","path"}, {}, {"intersections"});
    addBuiltin(db, "of",                C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "by",                C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "sort by",           C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "total",             C::Option, {}, {}, {}, {"intersections"});

    // Shape-specific options
    addBuiltin(db, "regular polygon sides", C::Option, {"tikzpicture","scope"}, {"node"},
               {"3","4","5","6","7","8","10","12"}, {"shapes.geometric"});
    addBuiltin(db, "star point ratio",      C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.3","0.4","0.5","0.6","0.8"}, {"shapes.geometric"});
    addBuiltin(db, "star point height",     C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "trapezium angle",       C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "aspect",                C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "shape border rotate",   C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "rectangle split parts", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});

    // Tree / graph
    addBuiltin(db, "grow",              C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow cyclic",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow=up",           C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow=down",         C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow=left",         C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow=right",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "level distance",    C::Option, {"tikzpicture","scope"},
               {"0.5cm","1cm","1.5cm","2cm","3cm"});
    addBuiltin(db, "sibling distance",  C::Option, {"tikzpicture","scope"},
               {"0.5cm","1cm","1.5cm","2cm","3cm"});
    addBuiltin(db, "branch",            C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "edge from parent",  C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "edge from parent path", C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "edge label",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "counterclockwise",  C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "clockwise",         C::Option, {"tikzpicture","scope"}, {"node"});

    // Mindmap
    addBuiltin(db, "mindmap",           C::Option, {"tikzpicture"});
    addBuiltin(db, "concept",           C::Option, {"tikzpicture"}, {}, {}, {"mindmap"});
    addBuiltin(db, "concept color",     C::Option, {"tikzpicture"}, {}, {}, {"mindmap"});

    // Automata
    addBuiltin(db, "accepting",         C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "initial",           C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "every state",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "loop above",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "loop below",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "loop left",         C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "loop right",        C::Option, {"tikzpicture","scope"});

    // Background layer
    addBuiltin(db, "on background layer",  C::Option, {"tikzpicture","scope"}, {}, {}, {"backgrounds"});
    addBuiltin(db, "show background rectangle", C::Option, {"tikzpicture","scope"}, {}, {}, {"backgrounds"});
    addBuiltin(db, "background rectangle", C::Option, {"tikzpicture","scope"}, {}, {}, {"backgrounds"});

    // Patterns as option
    addBuiltin(db, "pattern",           C::Option, {}, {"draw","path","fill","filldraw"},
               {"horizontal lines","vertical lines","north east lines","north west lines",
                "crosshatch","crosshatch dots","bricks","checkerboard",
                "grid","dots","fivepointed stars","sixpointed stars"});

    // Predefined styles
    addBuiltin(db, "help lines",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "information text",  C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "to path",           C::Option, {}, {"draw","path"});

    // Coordinate systems
    addBuiltin(db, "x",                 C::Option, {}, {});
    addBuiltin(db, "y",                 C::Option, {}, {});
    addBuiltin(db, "z",                 C::Option, {}, {});

    // ── tikz-3dplot options ──
    addBuiltin(db, "tdplot_main_coords",      C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "tdplot_rotated_coords",   C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "tdplot_screen_coords",    C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "canvas is xy plane at z", C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "canvas is xz plane at y", C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "canvas is yz plane at x", C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "canvas is plane",         C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "plane origin",            C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "theta",                   C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    addBuiltin(db, "phi",                     C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});

    // ── pgfplots axis options ──
    addBuiltin(db, "xlabel",            C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","polaraxis","groupplot"});
    addBuiltin(db, "ylabel",            C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","polaraxis","groupplot"});
    addBuiltin(db, "zlabel",            C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","polaraxis","groupplot"});
    addBuiltin(db, "xmin",              C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "xmax",              C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "ymin",              C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "ymax",              C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "zmin",              C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "zmax",              C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "xmode",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"},
               {}, {"normal","log"}, {});
    addBuiltin(db, "ymode",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"},
               {}, {"normal","log"}, {});
    addBuiltin(db, "zmode",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"},
               {}, {"normal","log"}, {});
    addBuiltin(db, "xtick",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "ytick",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "ztick",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "xticklabels",       C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "yticklabels",       C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "zticklabels",       C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "xstep",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "ystep",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "zstep",             C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "samples",           C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"},
               {}, {"50","100","200","500","1000"}, {});
    addBuiltin(db, "domain",            C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"},
               {}, {"0:10","0:1","-5:5","-10:10","-pi:pi","0:2*pi"}, {});
    addBuiltin(db, "only marks",        C::Option, {"axis"});
    addBuiltin(db, "no markers",        C::Option, {"axis"});
    addBuiltin(db, "mark",              C::Option, {"axis"}, {},
               {"*","x","+","-","|","o","asterisk","star","oplus","otimes",
                "square","square*","triangle","triangle*","diamond","diamond*",
                "pentagon","pentagon*","text","none"}, {});
    addBuiltin(db, "mark size",         C::Option, {"axis"}, {},
               {"1pt","2pt","3pt","4pt","5pt","6pt","8pt"}, {});
    addBuiltin(db, "mark options",      C::Option, {"axis"});
    addBuiltin(db, "sharp plot",        C::Option, {"axis"});
    addBuiltin(db, "const plot",        C::Option, {"axis"});
    addBuiltin(db, "error bars",        C::Option, {"axis"});
    addBuiltin(db, "title",             C::Option, {"axis","groupplot"});
    addBuiltin(db, "legend entries",    C::Option, {"axis","groupplot"});
    addBuiltin(db, "legend pos",        C::Option, {"axis","groupplot"}, {},
               {"north east","north west","south east","south west","outer north east"}, {});
    addBuiltin(db, "legend style",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "colormap",          C::Option, {"axis"});
    addBuiltin(db, "view",              C::Option, {"axis"}, {}, {}, {"3d"});
    addBuiltin(db, "axis equal",        C::Option, {"axis"});
    addBuiltin(db, "axis equal image",  C::Option, {"axis"});
    addBuiltin(db, "grid",              C::Option, {"axis"});
    addBuiltin(db, "grid style",        C::Option, {"axis"});
    addBuiltin(db, "major grid style",  C::Option, {"axis"});
    addBuiltin(db, "minor grid style",  C::Option, {"axis"});
    addBuiltin(db, "minor xtick",       C::Option, {"axis","groupplot"});
    addBuiltin(db, "minor ytick",       C::Option, {"axis","groupplot"});
    addBuiltin(db, "minor tick num",    C::Option, {"axis","groupplot"});
    addBuiltin(db, "x tick label style",C::Option, {"axis","groupplot"});
    addBuiltin(db, "y tick label style",C::Option, {"axis","groupplot"});
    addBuiltin(db, "enlargelimits",     C::Option, {"axis"});
    addBuiltin(db, "width",             C::Option, {"axis","groupplot"});
    addBuiltin(db, "height",            C::Option, {"axis","groupplot"});
    addBuiltin(db, "axis x line",       C::Option, {"axis"});
    addBuiltin(db, "axis y line",       C::Option, {"axis"});
    addBuiltin(db, "axis z line",       C::Option, {"axis"});
    addBuiltin(db, "axis line style",   C::Option, {"axis"});

    // ── CircuitikZ options ──
    addBuiltin(db, "american",     C::Option, {"circuitikz"}, {});
    addBuiltin(db, "european",     C::Option, {"circuitikz"}, {});
    addBuiltin(db, "wiper pos",    C::Option, {"circuitikz"}, {});

    // ── Execution hooks ──
    addBuiltin(db, "execute at begin picture", C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "execute at end picture",   C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "execute at begin scope",   C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "execute at end scope",     C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "execute at begin path",    C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "execute at end path",      C::Option, {"tikzpicture","scope"});

    // ── Blend mode ──
    addBuiltin(db, "blend mode", C::Option, {"tikzpicture","scope"}, {"draw","path","node","fill"},
               {"normal","multiply","screen","overlay","darken","lighten",
                "color dodge","color burn","hard light","soft light","difference",
                "exclusion","hue","saturation","color","luminosity"});

    // ── Path picture ──
    addBuiltin(db, "path picture",              C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "path picture bounding box", C::Option, {}, {"node"});
    addBuiltin(db, "label distance", C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","1pt","2pt","3pt","5pt","10pt"});

    // ── Every series ──
    addBuiltin(db, "every pic",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every coordinate", C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every to",         C::Option, {"tikzpicture","scope"});

    // ── Dash pattern ──
    addBuiltin(db, "dash pattern", C::Option, {}, {"draw","path"});
    addBuiltin(db, "dash expand",  C::Option, {}, {"draw","path"});
    addBuiltin(db, "transparency group", C::Option, {}, {"draw","path","node"},
               {"isolated","knockout"});

    // ── arrows.meta options ──
    addBuiltin(db, "arrows",      C::Option, {}, {"draw","path"});
    addBuiltin(db, "tips",        C::Option, {}, {"draw","path"},
               {"proper","on proper draw","no tips"});
    addBuiltin(db, "every arrow", C::Option, {"tikzpicture","scope"});

    // ── Callout shapes ──
    const char *calloutShapes[] = {
        "rectangle callout","ellipse callout","cloud callout",
        "line callout","rounded rectangle callout",nullptr};
    for (int i = 0; calloutShapes[i]; i++)
        addBuiltin(db, calloutShapes[i], C::Shape, {"tikzpicture","scope"}, {"node"},
                   {}, {"shapes.callouts"});

    // ── Symbol shapes ──
    const char *symbolShapes[] = {
        "tape","magnetic tape","forbidden sign","lightning","signal",nullptr};
    for (int i = 0; symbolShapes[i]; i++)
        addBuiltin(db, symbolShapes[i], C::Shape, {"tikzpicture","scope"}, {"node"},
                   {}, {"shapes.symbols"});

    // ── Misc shapes ──
    addBuiltin(db, "cross out",     C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.misc"});
    addBuiltin(db, "circle around", C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.misc"});
    addBuiltin(db, "strike out",    C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.misc"});
    addBuiltin(db, "rounded rectangle", C::Shape, {"tikzpicture","scope"}, {"node"});

    // ── CircuiTikZ node shapes ──
    auto ctikzShapeEnvs = {"tikzpicture","scope","circuitikz"};

    // Passive shapes
    addCtikzShapes(db, {
        "resistor","potentiometer","thermistor","varistor","photoresistor",
        "capacitor","elmech","elco","pvarcapacitor","vvarcapacitor",
        "inductor","cute_inductor","american_inductor","coils","cute_coil","american_coil",
        "memristor","generic","emptygeneric","fullgeneric",
    }, ctikzShapeEnvs);
    addCtikzShape(db, "vresistorsshape", ctikzShapeEnvs);
    addCtikzShape(db, "vcapacitorshape", ctikzShapeEnvs);
    addCtikzShape(db, "vcoilsshape", ctikzShapeEnvs);

    // Power shapes
    addCtikzShapes(db, {
        "vsourcesin","isourcesin","vsource","isource","vsourceDC","isourceDC",
        "battery1","battery2","solarcell",
    }, ctikzShapeEnvs);

    // Diode shapes
    addCtikzShapes(db, {
        "diode","led","zdiode","schottky","photodiode","varactor",
        "tunneldiode","thyristor","triac","diac",
    }, ctikzShapeEnvs);

    // Transistor shapes
    addCtikzShapes(db, {
        "npn","pnp",
        "nigfet","pigfet","nigfete","pigfete",
        "nmos","pmos","nmosfete","pmosfete",
        "njfet","pjfet",
        "nchenh","nchdep","pchdep","nch","pch",
        "nigmfet","pigmfet",
        "Lnigfet","Lpigfet",
    }, ctikzShapeEnvs);

    // Instrument shapes
    addCtikzShapes(db, {
        "ammeter","voltmeter","ohmmeter",
        "rmeter","rmeterwa","smeter",
        "qiprobe","qvprobe","qprobe",
        "iloop2","iloop","viscoe",
    }, ctikzShapeEnvs);

    // Switch shapes
    addCtikzShapes(db, {
        "switch","cspst","ospt",
        "cswitch","oswitch",
        "pushbutton","nopb","ncpb",
    }, ctikzShapeEnvs);

    // Mechanical shapes
    addCtikzShapes(db, {
        "motor","motor2","gear",
        "short","open",
        "twoport","fourport",
        "transformer","transformercore","gyrator",
        "ctline","delayline","tline",
    }, ctikzShapeEnvs);

    // Terminal shapes
    addCtikzShapes(db, {
        "ocirc","fcirc","ccirc",
        "ocorner","icorner",
        "ground","rground","cground","sground","tground","noground",
    }, ctikzShapeEnvs);

    // Logic gate shapes
    addCtikzShapes(db, {
        "american and gate","american or gate","american not gate",
        "american nand gate","american nor gate","american xor gate","american xnor gate",
        "american buffer gate","american inverter gate",
        "and port","or port","not port",
        "nand port","nor port","xor port","xnor port",
        "buffer port","inverter port",
    }, ctikzShapeEnvs);

    // Bipole prefix shapes (90 combinations)
    {
        static const char *prefixes[] = {
            "p","v","i","ld","s","q","o","vq","iq","qq", nullptr
        };
        static const char *bipoles[] = {
            "R","L","C","D","V","I","Q","Ty","Tr", nullptr
        };
        for (int p = 0; prefixes[p]; p++)
            for (int b = 0; bipoles[b]; b++) {
                QByteArray sn = QByteArray(prefixes[p]) + bipoles[b] + "shape";
                addCtikzShape(db, sn.constData(), ctikzShapeEnvs);
            }
    }

    // ── Decorations ──
    addBuiltin(db, "random steps",       C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathmorphing"});
    addBuiltin(db, "wave",               C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathmorphing"});
    addBuiltin(db, "bent",               C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathmorphing"});
    addBuiltin(db, "lineto",             C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathmorphing"});
    addBuiltin(db, "straight zigzag",    C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathmorphing"});
    addBuiltin(db, "text along path",       C::Decoration, {}, {"draw","path"}, {},
               {"decorations.text"});
    addBuiltin(db, "text effects along path", C::Decoration, {}, {"draw","path"}, {},
               {"decorations.text"});
    addBuiltin(db, "show path construction", C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathreplacing"});
    addBuiltin(db, "expanding waves",        C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathreplacing"});
    addBuiltin(db, "ticks",                  C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathreplacing"});
    addBuiltin(db, "border",                 C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathreplacing"});
    addBuiltin(db, "waves",                  C::Decoration, {}, {"draw","path"}, {},
               {"decorations.pathreplacing"});
    addBuiltin(db, "triangles",          C::Decoration, {}, {"draw","path"}, {},
               {"decorations.shapes"});
    addBuiltin(db, "shape backgrounds",  C::Decoration, {}, {"draw","path"}, {},
               {"decorations.shapes"});
    addBuiltin(db, "Koch curve",         C::Decoration, {}, {"draw","path"}, {},
               {"decorations.fractals"});
    addBuiltin(db, "Koch snowflake",     C::Decoration, {}, {"draw","path"}, {},
               {"decorations.fractals"});
    addBuiltin(db, "Cantor set",         C::Decoration, {}, {"draw","path"}, {},
               {"decorations.fractals"});
    addBuiltin(db, "footprints",     C::Decoration, {}, {"draw","path"}, {},
               {"decorations.footprints"});
    addBuiltin(db, "footprints of",   C::Decoration, {}, {"draw","path"},
               {"bird","dog","frog","gnu"}, {"decorations.footprints"});

    // ── shadows.blur options ──
    addBuiltin(db, "blur shadow",         C::Option, {}, {"node"}, {}, {"shadows.blur"});
    addBuiltin(db, "custom blur shadow",  C::Option, {}, {"node"}, {}, {"shadows.blur"});
    addBuiltin(db, "blur shadow scale",   C::Option, {}, {"node"}, {}, {"shadows.blur"});
    addBuiltin(db, "blur shadow xshift",  C::Option, {}, {"node"}, {}, {"shadows.blur"});
    addBuiltin(db, "blur shadow yshift",  C::Option, {}, {"node"}, {}, {"shadows.blur"});
    addBuiltin(db, "blur shadow opacity", C::Option, {}, {"node"}, {}, {"shadows.blur"});
    addBuiltin(db, "blur shadow radius",  C::Option, {}, {"node"}, {}, {"shadows.blur"});
    addBuiltin(db, "blur shadow steps",   C::Option, {}, {"node"}, {}, {"shadows.blur"});

    // ── fadings options ──
    addBuiltin(db, "path fading",            C::Option, {}, {"draw","path","node"},
               {}, {"fadings"});
    addBuiltin(db, "path fading fade across", C::Option, {}, {"draw","path"}, {},
               {"fadings"});
    addBuiltin(db, "fade transform",          C::Option, {}, {"draw","path"}, {},
               {"fadings"});
    addBuiltin(db, "scope fading",            C::Option, {"tikzpicture","scope"}, {},
               {}, {"fadings"});

    // ── pgfplots grid options ──
    addBuiltin(db, "xmajorgrids",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "ymajorgrids",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "zmajorgrids",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "xminorgrids",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "yminorgrids",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "zminorgrids",      C::Option, {"axis","groupplot"});

    // ── pgfplots axis directions ──
    addBuiltin(db, "x dir", C::Option, {"axis","groupplot"}, {},
               {"normal","reverse"});
    addBuiltin(db, "y dir", C::Option, {"axis","groupplot"}, {},
               {"normal","reverse"});
    addBuiltin(db, "z dir", C::Option, {"axis","groupplot"}, {},
               {"normal","reverse"});

    // ── pgfplots axis style ──
    addBuiltin(db, "axis on top",          C::Option, {"axis","groupplot"});
    addBuiltin(db, "axis background",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "separate axis lines",  C::Option, {"axis","groupplot"});

    // ── pgfplots label styles ──
    addBuiltin(db, "xlabel style",       C::Option, {"axis","groupplot"});
    addBuiltin(db, "ylabel style",       C::Option, {"axis","groupplot"});
    addBuiltin(db, "zlabel style",       C::Option, {"axis","groupplot"});
    addBuiltin(db, "title style",        C::Option, {"axis","groupplot"});
    addBuiltin(db, "zticklabel style",   C::Option, {"axis","groupplot"});
    addBuiltin(db, "tick label style",   C::Option, {"axis","groupplot"});
    addBuiltin(db, "tick style",         C::Option, {"axis","groupplot"});
    addBuiltin(db, "tick align",         C::Option, {"axis","groupplot"},
               {"inside","outside","center"});

    // ── pgfplots scaled ticks ──
    addBuiltin(db, "scaled x ticks", C::Option, {"axis","groupplot"});
    addBuiltin(db, "scaled y ticks", C::Option, {"axis","groupplot"});
    addBuiltin(db, "scaled z ticks", C::Option, {"axis","groupplot"});

    // ── pgfplots log basis ──
    addBuiltin(db, "log basis x", C::Option, {"axis","semilogxaxis","loglogaxis","groupplot"});
    addBuiltin(db, "log basis y", C::Option, {"axis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "log basis z", C::Option, {"axis","groupplot"});

    // ── pgfplots bar chart ──
    addBuiltin(db, "bar width",     C::Option, {"axis","groupplot"});
    addBuiltin(db, "bar shift",     C::Option, {"axis","groupplot"});
    addBuiltin(db, "bar gap",       C::Option, {"axis","groupplot"});
    addBuiltin(db, "ybar",          C::Option, {"axis","groupplot"});
    addBuiltin(db, "xbar",          C::Option, {"axis","groupplot"});
    addBuiltin(db, "ybar stacked",  C::Option, {"axis","groupplot"});
    addBuiltin(db, "xbar stacked",  C::Option, {"axis","groupplot"});
    addBuiltin(db, "stack plots",   C::Option, {"axis","groupplot"},
               {"plus","minus"});

    // ── pgfplots legend ──
    addBuiltin(db, "area legend",       C::Option, {"axis","groupplot"});
    addBuiltin(db, "legend cell align", C::Option, {"axis","groupplot"},
               {"left","center","right"});
    addBuiltin(db, "legend columns",    C::Option, {"axis","groupplot"});
    addBuiltin(db, "legend image style",C::Option, {"axis","groupplot"});

    // ── pgfplots clip ──
    addBuiltin(db, "clip mode", C::Option, {"axis","groupplot"},
               {"default","individual"});
    addBuiltin(db, "clip",      C::Option, {"axis","groupplot"}, {},
               {"true","false"});

    // ── pgfplots colormap ──
    addBuiltin(db, "colormap name", C::Option, {"axis"},
               {"viridis","hot","cool","jet","blackwhite","greenyellow",
                "redyellow","violet","bluered"});

    // ── pgfplots plot styles ──
    addBuiltin(db, "smooth",    C::Option, {"axis"});
    addBuiltin(db, "area style",C::Option, {"axis","groupplot"});
    addBuiltin(db, "histogram", C::Option, {"axis","groupplot"});

    // ── pgfplots mesh/contour/quiver ──
    addBuiltin(db, "mesh",             C::Option, {"axis"});
    addBuiltin(db, "contour",          C::Option, {"axis"});
    addBuiltin(db, "contour prepared", C::Option, {"axis"});
    addBuiltin(db, "quiver",           C::Option, {"axis"});

    // ── pgfplots error bars ──
    addBuiltin(db, "error bars/.cd",  C::Option, {"axis"});
    addBuiltin(db, "error bar style", C::Option, {"axis"});
    addBuiltin(db, "error mark",      C::Option, {"axis"});

    // ── pgfplots addplot options ──
    addBuiltin(db, "domain y",   C::Option, {"axis","groupplot"});
    addBuiltin(db, "y domain",   C::Option, {"axis","groupplot"});
    addBuiltin(db, "samples y",  C::Option, {"axis","groupplot"});
    addBuiltin(db, "variable",   C::Option, {"axis","groupplot"});
    addBuiltin(db, "variable y", C::Option, {"axis","groupplot"});
    addBuiltin(db, "parametric", C::Option, {"axis","groupplot"});

    // ── pgfplots const plot variants ──
    addBuiltin(db, "const plot mark left",  C::Option, {"axis"});
    addBuiltin(db, "const plot mark right", C::Option, {"axis"});
    addBuiltin(db, "const plot mark mid",   C::Option, {"axis"});

    // ── CircuitikZ options (using requiredLibs filtering) ──

    // ── CircuitikZ options ──
    addBuiltin(db, "bipoles/length",     C::Option, {"circuitikz"});
    addBuiltin(db, "tripoles/mos style", C::Option, {"circuitikz"});
    addBuiltin(db, "nodes width",        C::Option, {"circuitikz"});
    addBuiltin(db, "current/distance",   C::Option, {"circuitikz"});
    addBuiltin(db, "voltage/distance",   C::Option, {"circuitikz"});
    addBuiltin(db, "voltage/shift",      C::Option, {"circuitikz"});
    addBuiltin(db, "current/shift",      C::Option, {"circuitikz"});
    addBuiltin(db, "label/align",        C::Option, {"circuitikz"});
    addBuiltin(db, "thickness",          C::Option, {"circuitikz"});
    addBuiltin(db, "l",   C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "l_",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "l^",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "a",   C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "v",   C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "v_",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "v^",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "i",   C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "i_",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "i^",  C::Option, {"circuitikz"}, {"draw","path"});

    // ── tikz-cd options ──
    addBuiltin(db, "from",          C::Option, {"tikzcd"});
    addBuiltin(db, "to",            C::Option, {"tikzcd"});
    addBuiltin(db, "phantom",       C::Option, {"tikzcd"}, {"node"});
    addBuiltin(db, "description",   C::Option, {"tikzcd"});
    addBuiltin(db, "cramped",       C::Option, {"tikzcd"});
    addBuiltin(db, "shift left",    C::Option, {"tikzcd"});
    addBuiltin(db, "shift right",   C::Option, {"tikzcd"});
    addBuiltin(db, "crossing over", C::Option, {"tikzcd"});
    addBuiltin(db, "math nodes",    C::Option, {"tikzcd"});
    addBuiltin(db, "cells",         C::Option, {"tikzcd"});
    addBuiltin(db, "arrow style",   C::Option, {"tikzcd"},
               {"latin","tikz","knuth","computermodern"});
    addBuiltin(db, "labels",        C::Option, {"tikzcd"});
    addBuiltin(db, "tweak",         C::Option, {"tikzcd"});

    // ── tikz-cd commands ──
    addBuiltin(db, "arrow",        C::Command, {"tikzcd"});
    addBuiltin(db, "rar",          C::Command, {"tikzcd"});
    addBuiltin(db, "lar",          C::Command, {"tikzcd"});
    addBuiltin(db, "uar",          C::Command, {"tikzcd"});
    addBuiltin(db, "dar",          C::Command, {"tikzcd"});
    addBuiltin(db, "obj",          C::Command, {"tikzcd"});

    // ── graphs library ──
    addBuiltin(db, "subgraph",       C::Option, {}, {"draw","path"}, {},
               {"graphs.standard"});
    addBuiltin(db, "subgraph K_n",   C::Option, {}, {"draw","path"}, {},
               {"graphs.standard"});
    addBuiltin(db, "subgraph C_n",   C::Option, {}, {"draw","path"}, {},
               {"graphs.standard"});
    addBuiltin(db, "subgraph I_n",   C::Option, {}, {"draw","path"}, {},
               {"graphs.standard"});
    addBuiltin(db, "subgraph P_n",   C::Option, {}, {"draw","path"}, {},
               {"graphs.standard"});
    addBuiltin(db, "simple",         C::Option, {}, {"draw","path"}, {},
               {"graphs"});
    addBuiltin(db, "clique",         C::Option, {}, {"draw","path"}, {},
               {"graphs"});
    addBuiltin(db, "cycle",          C::Option, {}, {"draw","path"}, {},
               {"graphs"});
    addBuiltin(db, "grid",           C::Option, {}, {"draw","path"}, {},
               {"graphs"});

    // ── foreach options ──
    addBuiltin(db, "evaluate", C::Option, {}, {"foreach"});
    addBuiltin(db, "parse",    C::Option, {}, {"foreach"});
    addBuiltin(db, "remember", C::Option, {}, {"foreach"});

    // ── fit library ──
    addBuiltin(db, "fit",         C::Option, {"tikzpicture","scope"}, {"node"},
               {}, {"fit"});
    addBuiltin(db, "fit margins", C::Option, {"tikzpicture","scope"}, {"node"},
               {}, {"fit"});

    // ── chains library ──
    addBuiltin(db, "start chain",     C::Option, {"tikzpicture","scope"}, {},
               {}, {"chains"});
    addBuiltin(db, "on chain",        C::Option, {}, {"node"}, {}, {"chains"});
    addBuiltin(db, "join",            C::Option, {}, {"node"}, {}, {"chains"});

    // ── angles library ──
    addBuiltin(db, "angle",     C::Command, {}, {"draw","path","pic"}, {},
               {"angles"});
    addBuiltin(db, "pic type",  C::Option, {}, {"pic"},
               {"angle","right angle"}, {"angles"});

    // ── Additional libraries ──
    addBuiltin(db, "shadows.blur",         C::Library);
    addBuiltin(db, "graphs.simple",        C::Library);
    addBuiltin(db, "pgfplots.fillbetween", C::Library);

    // ── Additional commands ──
    addBuiltin(db, "addplot+",           C::Command, {"axis","groupplot"});
    addBuiltin(db, "addplot table",      C::Command, {"axis","groupplot"});
    addBuiltin(db, "addplot coordinates",C::Command, {"axis","groupplot"});
    addBuiltin(db, "addplot graphics",   C::Command, {"axis","groupplot"});
    addBuiltin(db, "addplot3 table",     C::Command, {"axis","groupplot"});
    addBuiltin(db, "pgfplotstableset",   C::Command);
    addBuiltin(db, "pgfplotstabletypeset", C::Command);
    addBuiltin(db, "draw plot",          C::Command);
    addBuiltin(db, "node at",            C::Command);
    addBuiltin(db, "coordinate at",      C::Command);
    addBuiltin(db, "tikzmath",           C::Command);
    addBuiltin(db, "Pgfmathsetmacro",    C::Command);
    addBuiltin(db, "Pgfmathsetlength",   C::Command);
    addBuiltin(db, "pgfmathparse",       C::Command);
    addBuiltin(db, "pgfmathresult",      C::Command);
    addBuiltin(db, "pgfplotstableread",  C::Command);

    // Build name index
    for (int i = 0; i < db.size(); i++) {
        m_nameIndex[makeKey(db[i].name, db[i].category)] = i;
    }
}

TikzKeywordDB::TikzKeywordDB()
{
    initBuiltins();
}

TikzKeywordDB &TikzKeywordDB::instance()
{
    static TikzKeywordDB db;
    return db;
}

QString TikzKeywordDB::makeKey(const QString &name, Category cat) const
{
    return name.toLower() + QChar('|') + QString::number(static_cast<int>(cat));
}

QVector<const TikzKeyword*> TikzKeywordDB::filter(
    const QString &contextEnv,
    const QString &contextCmd,
    const QSet<QString> &activeLibs,
    Category cat, bool includeUserDefined) const
{
    QVector<const TikzKeyword*> result;
    QString envLower = contextEnv.toLower();
    QString cmdLower = contextCmd.toLower();

    auto matches = [&](const TikzKeyword &kw) -> bool {
        if (kw.category != cat) return false;
        if (!kw.environments.isEmpty() && !kw.environments.contains(envLower)
            && !kw.environments.contains("tikzpicture") && !kw.environments.contains("scope"))
            return false;
        if (!kw.commands.isEmpty() && !cmdLower.isEmpty()) {
            bool cmdMatch = kw.commands.contains(cmdLower);
            if (!cmdMatch) {
                static const QSet<QString> pathCmds = {"draw","path","fill","filldraw","shade","shadedraw","edge","to"};
                static const QSet<QString> nodeCmds = {"node","pic"};
                bool isPathCmd = pathCmds.contains(cmdLower);
                bool hasNodeCmd = kw.commands.intersects(nodeCmds);
                if (!(isPathCmd && hasNodeCmd))
                    return false;
            }
        }
        if (!kw.requiredLibs.isEmpty()) {
            bool found = false;
            for (const QString &lib : kw.requiredLibs) {
                if (activeLibs.contains(lib)) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    };

    for (const auto &kw : m_builtin) {
        if (matches(kw)) result.append(&kw);
    }
    if (includeUserDefined) {
        for (const auto &kw : m_userDefined) {
            if (matches(kw)) result.append(&kw);
        }
    }
    return result;
}

const TikzKeyword *TikzKeywordDB::find(const QString &name, Category cat) const
{
    QString key = makeKey(name, cat);
    if (m_nameIndex.contains(key)) {
        return &m_builtin[m_nameIndex[key]];
    }
    for (const auto &kw : m_userDefined) {
        if (kw.name.compare(name, Qt::CaseInsensitive) == 0 && kw.category == cat)
            return &kw;
    }
    return nullptr;
}

QStringList TikzKeywordDB::names(Category cat) const
{
    QStringList result;
    for (const auto &kw : m_builtin)
        if (kw.category == cat) result << kw.name;
    for (const auto &kw : m_userDefined)
        if (kw.category == cat) result << kw.name;
    result.removeDuplicates();
    result.sort(Qt::CaseInsensitive);
    return result;
}

QStringList TikzKeywordDB::allOptionNames() const
{
    return names(Category::Option)
        + names(Category::Color)
        + names(Category::LineWidth)
        + names(Category::LineType)
        + names(Category::Arrow)
        + names(Category::Pattern)
        + names(Category::Decoration)
        + names(Category::Shape);
}

QStringList TikzKeywordDB::allAnchorNames() const { return names(Category::Anchor); }
QStringList TikzKeywordDB::allColorNames() const { return names(Category::Color); }
QStringList TikzKeywordDB::allArrowNames() const { return names(Category::Arrow); }
QStringList TikzKeywordDB::allLineTypeNames() const { return names(Category::LineType); }
QStringList TikzKeywordDB::allLineWidthNames() const { return names(Category::LineWidth); }
QStringList TikzKeywordDB::allCommandNames() const { return names(Category::Command); }
QStringList TikzKeywordDB::allEnvNames() const { return names(Category::Environment); }
QStringList TikzKeywordDB::allLibNames() const { return names(Category::Library); }
QStringList TikzKeywordDB::allHandlerNames() const { return names(Category::Handler); }
QStringList TikzKeywordDB::allMathFuncNames() const { return names(Category::MathFunction); }
QStringList TikzKeywordDB::allPatternNames() const { return names(Category::Pattern); }
QStringList TikzKeywordDB::allDecorationNames() const { return names(Category::Decoration); }

QStringList TikzKeywordDB::allCompletableWords() const
{
    QStringList result;
    QSet<QString> seen;
    auto appendUnique = [&](const QStringList &words) {
        for (const QString &w : words) {
            QString lower = w.toLower();
            if (!seen.contains(lower)) {
                seen.insert(lower);
                result << w;
            }
        }
    };
    appendUnique(allCommandNames());
    appendUnique(allOptionNames());
    appendUnique(allAnchorNames());
    appendUnique(allColorNames());
    appendUnique(allArrowNames());
    appendUnique(allLineTypeNames());
    appendUnique(allLineWidthNames());
    appendUnique(allMathFuncNames());
    result.sort(Qt::CaseInsensitive);
    return result;
}

QVector<QPair<QString, QStringList>> TikzKeywordDB::allValueHints() const
{
    QVector<QPair<QString, QStringList>> hints;
    QSet<QString> seenKeys;
    for (const auto &kw : m_builtin) {
        if (!kw.valueHints.isEmpty() && !seenKeys.contains(kw.name.toLower())) {
            seenKeys.insert(kw.name.toLower());
            hints.append({kw.name, kw.valueHints});
        }
    }
    // Add common value hints for color and arrow keys
    QStringList colorHints = allColorNames();
    QStringList arrowHints = allArrowNames();
    QStringList lineWidthHints = names(Category::LineWidth);
    auto addHint = [&](const QString &key, const QStringList &vals, bool &added) {
        if (seenKeys.contains(key.toLower())) return;
        seenKeys.insert(key.toLower());
        hints.append({key, vals});
    };
    bool dummy = false;
    addHint("color", colorHints, dummy);
    addHint("draw", colorHints, dummy);
    addHint("fill", colorHints, dummy);
    addHint("left color", colorHints, dummy);
    addHint("right color", colorHints, dummy);
    addHint("middle color", colorHints, dummy);
    addHint("top color", colorHints, dummy);
    addHint("bottom color", colorHints, dummy);
    addHint("inner color", colorHints, dummy);
    addHint("outer color", colorHints, dummy);
    addHint("ball color", colorHints, dummy);
    addHint("pattern color", colorHints, dummy);
    addHint("concept color", colorHints, dummy);
    addHint("arrow", arrowHints, dummy);
    addHint(">", arrowHints, dummy);
    addHint(">=", arrowHints, dummy);
    addHint("-", arrowHints, dummy);
    addHint("line width", {"0.2pt","0.4pt","0.5pt","0.6pt","0.8pt","1pt","1.2pt","1.5pt","2pt","2.5pt","3pt","4pt","5pt","6pt","8pt","10pt"}, dummy);
    return hints;
}

QStringList TikzKeywordDB::valueHintsFor(const QString &keyName) const
{
    for (const auto &kw : m_builtin) {
        if (kw.name.compare(keyName, Qt::CaseInsensitive) == 0 && !kw.valueHints.isEmpty())
            return kw.valueHints;
    }
    for (const auto &pair : allValueHints()) {
        if (pair.first.compare(keyName, Qt::CaseInsensitive) == 0)
            return pair.second;
    }
    return {};
}

void TikzKeywordDB::registerUserDefined(const QString &name, Category cat,
                                         const QString &doc)
{
    // Avoid duplicates
    for (auto &kw : m_userDefined) {
        if (kw.name == name && kw.category == cat) {
            kw.doc = doc;
            return;
        }
    }
    TikzKeyword kw;
    kw.name = name;
    kw.category = cat;
    kw.doc = doc;
    m_userDefined.append(kw);
}

void TikzKeywordDB::clearUserDefined()
{
    m_userDefined.clear();
}

} // namespace TikzKeywords
