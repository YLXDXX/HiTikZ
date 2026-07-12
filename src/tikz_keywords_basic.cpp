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

    // ── Classic arrow tip names ── (pgflibraryarrows, no arrows.meta needed)
    // Capitalized arrows.meta tips (Stealth/Latex/Circle/...) are registered in
    // registerExtended() gated by \usetikzlibrary{arrows.meta}.
    const char *arrows[] = {"stealth","stealth'","latex","latex'","latex reversed","to","to reversed",
        "angle 90","angle 60","angle 45","triangle 90","triangle 60","triangle 45",
        "open triangle 90","open triangle 60","open triangle 45",
        "left to","right to","left hook","right hook","hooks",
        "round cap","butt cap","fast cap","serif cm","implies",
        "o","*","diamond","open diamond","square","open square",nullptr};
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
    // Names and library attributions verified against
    // texmf-dist/tex/generic/pgf/ (\pgfdeclaredecoration / \pgfdeclaremetadecoration).
    // pathmorphing:
    addBuiltin(db, "snake",    C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "coil",     C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "saw",      C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "zigzag",   C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "bumps",    C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "bent",     C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "random steps", C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    addBuiltin(db, "straight zigzag", C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathmorphing"});
    // core built-in decorations (no library needed):
    addBuiltin(db, "moveto",   C::Decoration, {}, {"draw","path"});
    addBuiltin(db, "lineto",   C::Decoration, {}, {"draw","path"});
    addBuiltin(db, "curveto",  C::Decoration, {}, {"draw","path"});
    // pathreplacing:
    addBuiltin(db, "brace",        C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "ticks",        C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "waves",        C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "expanding waves", C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "border",       C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    addBuiltin(db, "show path construction", C::Decoration, {}, {"draw","path"}, {}, {"decorations.pathreplacing"});
    // markings:
    addBuiltin(db, "markings", C::Decoration, {}, {"draw","path"}, {}, {"decorations.markings"});
    // fractals (exact names from the fractals library):
    addBuiltin(db, "Koch snowflake",    C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Koch curve type 1", C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Koch curve type 2", C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    addBuiltin(db, "Cantor set",        C::Decoration, {}, {"draw","path"}, {}, {"decorations.fractals"});
    // text / footprints / shapes:
    addBuiltin(db, "text along path", C::Decoration, {}, {"draw","path"}, {}, {"decorations.text"});
    addBuiltin(db, "footprints",      C::Decoration, {}, {"draw","path"}, {}, {"decorations.footprints"});
    addBuiltin(db, "crosses",         C::Decoration, {}, {"draw","path"}, {}, {"decorations.shapes"});
    addBuiltin(db, "triangles",       C::Decoration, {}, {"draw","path"}, {}, {"decorations.shapes"});
    addBuiltin(db, "shape backgrounds", C::Decoration, {}, {"draw","path"}, {}, {"decorations.shapes"});
}

void registerAnchors(Vec &db)
{
    // ── Universal PGF anchors (always available) ──
    // Verified against pgf/modules/pgfmoduleshapes.code.tex (TeXLive).
    const char *universal[] = {
        "north","north east","north west","south","south east","south west",
        "east","west","center","text",
        "base","base east","base west",
        "mid","mid east","mid west",
        nullptr};
    for (int i = 0; universal[i]; i++)
        addBuiltin(db, universal[i], C::Anchor, {}, {"node"});

    // ── shapes.multipart (circle split / rectangle split) ──
    // Verified against pgflibraryshapes.multipart.code.tex.
    const char *multipart[] = {
        "lower",                    // circle split lower half
        nullptr};
    for (int i = 0; multipart[i]; i++)
        addBuiltin(db, multipart[i], C::Anchor, {}, {"node"}, {},
                   {"shapes.multipart"});

    // ── shapes.geometric ──
    // Verified against pgflibraryshapes.geometric.code.tex.
    const char *geometric[] = {
        "apex",                     // isosceles triangle
        nullptr};
    for (int i = 0; geometric[i]; i++)
        addBuiltin(db, geometric[i], C::Anchor, {}, {"node"}, {},
                   {"shapes.geometric"});

    // ── shapes.symbols / shapes.arrows ──
    // Verified against pgflibraryshapes.symbols/arrows.code.tex.
    {
        addBuiltin(db, "tail", C::Anchor, {}, {"node"}, {}, {"shapes.arrows"});
    }

    // ── shapes.gates.logic.US / .IEC ──
    // Verified against pgflibraryshapes.gates.(logic.US|logic.IEC|ee).code.tex.
    {
        addBuiltin(db, "input", C::Anchor, {}, {"node"}, {},
                   {"shapes.gates.logic.US"});
        addBuiltin(db, "output", C::Anchor, {}, {"node"}, {},
                   {"shapes.gates.logic.US"});
    }

    // ── CircuiTikZ (require circuitikz library) ──
    // Verified against pgfcirc{bipoles,tripoles,multipoles,quadpoles}.tex.
    const char *circuitikz[] = {
        "wiper","W",
        "cathode","anode","gate","G",
        "in","in 1","in 2","out","out 1","out 2",
        "tap","tap down","tap up",
        "v+","v-",
        "tip",
        "B","C","E","S","D",
        "collector","emitter","source","drain","bulk",
        "+","-",
        "left","right","top","bottom",
        nullptr};
    for (int i = 0; circuitikz[i]; i++)
        addBuiltin(db, circuitikz[i], C::Anchor, {}, {"node"}, {},
                   {"circuitikz"});
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
        "chemfig","tikz-feynman","feynman",
        "shapes.gates.logic","shapes.gates.logic.IEC","shapes.gates.logic.US",
        nullptr};
    for (int i = 0; libs[i]; i++)
        addBuiltin(db, libs[i], C::Library);
}

// TikZ coordinate systems, usable as "(name cs:key=value,...)". Verified
// against the PGF/TikZ 3.1.10 sources:
//   • core systems (\tikzdeclarecoordinatesystem in tikz.code.tex): canvas,
//     canvas polar, xyz, xyz polar (alias xy polar), node, barycentric,
//     intersection, perpendicular
//   • 3d library (tikzlibrary3d.code.tex): xyz cylindrical, xyz spherical
//   • calc library (tikzlibrarycalc.code.tex): tangent
//   • perspective library (tikzlibraryperspective.code.tex): three point
//     perspective (alias tpp)
// The per-system option keys (stored under /tikz/cs/...) are attached as
// valueHints so they can be offered once the user is inside "cs:".
void registerCoordinateSystems(Vec &db)
{
    // Core systems (always available, no library gating).
    addBuiltin(db, "canvas", C::CoordinateSystem, {}, {}, {"x","y"});
    addBuiltin(db, "canvas polar", C::CoordinateSystem, {}, {},
               {"angle","radius","x radius","y radius"});
    addBuiltin(db, "xyz", C::CoordinateSystem, {}, {}, {"x","y","z"});
    addBuiltin(db, "xyz polar", C::CoordinateSystem, {}, {},
               {"angle","radius","x radius","y radius"});
    addBuiltin(db, "xy polar", C::CoordinateSystem, {}, {},
               {"angle","radius","x radius","y radius"});
    addBuiltin(db, "node", C::CoordinateSystem, {}, {},
               {"name","anchor","angle"});
    addBuiltin(db, "barycentric", C::CoordinateSystem);
    addBuiltin(db, "intersection", C::CoordinateSystem, {}, {},
               {"first line","second line","first node","second node",
                "solution","horizontal line through","vertical line through"});
    addBuiltin(db, "perpendicular", C::CoordinateSystem, {}, {},
               {"horizontal line through","vertical line through"});

    // 3d library: cylindrical/spherical coordinate systems.
    addBuiltin(db, "xyz cylindrical", C::CoordinateSystem, {}, {},
               {"angle","radius","z"}, {"3d"});
    addBuiltin(db, "xyz spherical", C::CoordinateSystem, {}, {},
               {"angle","radius","latitude","longitude"}, {"3d"});

    // calc library: tangent to a shape.
    addBuiltin(db, "tangent", C::CoordinateSystem, {}, {},
               {"node","point"}, {"calc"});

    // perspective library: three point perspective.
    addBuiltin(db, "three point perspective", C::CoordinateSystem, {}, {},
               {"x","y","z"}, {"perspective"});
    addBuiltin(db, "tpp", C::CoordinateSystem, {}, {},
               {"x","y","z"}, {"perspective"});
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
