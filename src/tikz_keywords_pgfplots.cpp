#include "tikz_keywords_data.h"

namespace TikzKeywords {

void registerPgfplotsAndShapes(Vec &db)
{
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
    // 'american'/'european' are valid global styles (\tikzset{american/.style=...}
    // in circuitikz); keep them. Also offer the choice-key forms voltage=/current=.
    addBuiltin(db, "american",     C::Option, {"circuitikz"}, {});
    addBuiltin(db, "european",     C::Option, {"circuitikz"}, {});
    addBuiltin(db, "voltage",      C::Option, {"circuitikz"}, {},
               {"european","american","straight","raised","curved"});
    addBuiltin(db, "current",      C::Option, {"circuitikz"}, {},
               {"european","american"});
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
    addBuiltin(db, "strike out",    C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.misc"});
    addBuiltin(db, "rounded rectangle", C::Shape, {"tikzpicture","scope"}, {"node"});
}

} // namespace TikzKeywords
