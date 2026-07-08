#include "tikz_keywords_data.h"

namespace TikzKeywords {

void registerColors(Vec &db)
{
    // ── Colors ──
    const char *allColors[] = {"red","green","blue","cyan","magenta","yellow","black","white",
        "gray","darkgray","lightgray","brown","orange","purple","violet","pink","olive","teal","lime",
        "darkblue","darkgreen","darkred","darkcyan","darkmagenta","darkyellow","darkorange",
        "darkpurple","darkviolet","lightblue","lightgreen","lightred","lightcyan","lightmagenta",
        "lightyellow","lightorange","lightpink","copper","gold","silver","bronze",nullptr};
    for (int i = 0; allColors[i]; i++)
        addBuiltin(db, allColors[i], C::Color, {"tikzpicture","scope","axis","circuitikz"},
                   {"draw","path","fill","filldraw","shade","shadedraw","node"});
}

void registerLineWidths(Vec &db)
{
    // ── Line widths ──
    addBuiltin(db, "ultra thin",  C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "very thin",   C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "thin",        C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "semithick",   C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "thick",       C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "very thick",  C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
    addBuiltin(db, "ultra thick", C::LineWidth, {"tikzpicture","scope","axis"}, {"draw","path","fill","filldraw"});
}

void registerLineTypes(Vec &db)
{
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
}

void registerArrows(Vec &db)
{
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
}

void registerShapes(Vec &db)
{
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
}

void registerPatterns(Vec &db)
{
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
}

void registerDecorations(Vec &db)
{
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
}

void registerAnchors(Vec &db)
{
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
}

void registerHandlers(Vec &db)
{
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
}

void registerPgfKeyPaths(Vec &db)
{
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
}

void registerLibraries(Vec &db)
{
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
}

void registerEnvironments(Vec &db)
{
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
}

} // namespace TikzKeywords
