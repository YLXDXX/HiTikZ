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
    addBuiltin(db, "xtick distance",    C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "ytick distance",    C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "ztick distance",    C::Option, {"axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"});
    addBuiltin(db, "samples",           C::Option, {"tikzpicture","scope","axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"},
               {"draw","path"}, {"50","100","200","500","1000"}, {});
    addBuiltin(db, "samples at",        C::Option, {"tikzpicture","scope","axis","groupplot"}, {"draw","path"});
    addBuiltin(db, "domain",            C::Option, {"tikzpicture","scope","axis","semilogxaxis","semilogyaxis","loglogaxis","groupplot"},
               {"draw","path"}, {"0:10","0:1","-5:5","-10:10","-pi:pi","0:2*pi"}, {});
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
    addBuiltin(db, "grid",              C::Option, {"axis"}, {},
               {"none","major","minor","both"});
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
    addBuiltin(db, "axis x line*",      C::Option, {"axis"});
    addBuiltin(db, "axis y line*",      C::Option, {"axis"});
    addBuiltin(db, "axis line style",   C::Option, {"axis"});
    addBuiltin(db, "axis lines",        C::Option, {"axis"}, {},
               {"box","left","middle","center","right","none"});
    addBuiltin(db, "axis lines*",       C::Option, {"axis"}, {},
               {"box","left","middle","center","right","none"});

    // ── pgfplots keys (verified in pgfplots 1.18.1 pgfplots.code.tex) ──
    addBuiltin(db, "enlarge x limits", C::Option, {"axis","groupplot"},
               {}, {"true","false","0.1","0.2","upper","lower"});
    addBuiltin(db, "enlarge y limits", C::Option, {"axis","groupplot"},
               {}, {"true","false","0.1","0.2","upper","lower"});
    addBuiltin(db, "enlarge z limits", C::Option, {"axis","groupplot"});
    addBuiltin(db, "xlabel near ticks", C::Option, {"axis","groupplot"});
    addBuiltin(db, "ylabel near ticks", C::Option, {"axis","groupplot"});
    addBuiltin(db, "restrict x to domain", C::Option, {"axis","groupplot"});
    addBuiltin(db, "restrict y to domain", C::Option, {"axis","groupplot"});
    addBuiltin(db, "colorbar",         C::Option, {"axis"});
    addBuiltin(db, "colorbar horizontal", C::Option, {"axis"});
    addBuiltin(db, "colorbar right",   C::Option, {"axis"});
    addBuiltin(db, "colorbar style",   C::Option, {"axis"});
    addBuiltin(db, "point meta",       C::Option, {"axis"}, {},
               {"x","y","z","f(x)","explicit","explicit symbolic","none"});
    addBuiltin(db, "point meta min",   C::Option, {"axis"});
    addBuiltin(db, "point meta max",   C::Option, {"axis"});
    addBuiltin(db, "scatter src",      C::Option, {"axis"}, {},
               {"x","y","z","explicit","explicit symbolic"});
    addBuiltin(db, "scatter",          C::Option, {"axis"});
    addBuiltin(db, "scatter/use mapped color", C::Option, {"axis"});
    addBuiltin(db, "forget plot",      C::Option, {"axis"});
    addBuiltin(db, "each nth point",   C::Option, {"axis"});
    addBuiltin(db, "nodes near coords", C::Option, {"axis"});
    addBuiltin(db, "nodes near coords style", C::Option, {"axis"});
    addBuiltin(db, "scale only axis",  C::Option, {"axis","groupplot"});
    addBuiltin(db, "x filter",         C::Option, {"axis"});
    addBuiltin(db, "y filter",         C::Option, {"axis"});
    addBuiltin(db, "extra x ticks",    C::Option, {"axis","groupplot"});
    addBuiltin(db, "extra y ticks",    C::Option, {"axis","groupplot"});
    addBuiltin(db, "hide axis",        C::Option, {"axis","groupplot"});
    addBuiltin(db, "hide x axis",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "hide y axis",      C::Option, {"axis","groupplot"});
    addBuiltin(db, "shader",           C::Option, {"axis"}, {},
               {"flat","flat corner","flat mean","interp","faceted","faceted interp"});
    addBuiltin(db, "unbounded coords", C::Option, {"axis"}, {},
               {"discard","jump"});
    addBuiltin(db, "unit vector ratio", C::Option, {"axis"});
    addBuiltin(db, "compat",           C::Option, {"axis","groupplot"}, {},
               {"newest","1.18","1.17","1.16","1.15","1.14","1.13","1.12","1.11","1.10","1.9","1.8","1.7","1.6","1.5","1.4","1.3"});
    addBuiltin(db, "cycle list name",  C::Option, {"axis","groupplot"});
    addBuiltin(db, "cycle list",       C::Option, {"axis","groupplot"});
    addBuiltin(db, "xtick pos",        C::Option, {"axis","groupplot"}, {},
               {"left","right","both","lower","upper"});
    addBuiltin(db, "ytick pos",        C::Option, {"axis","groupplot"}, {},
               {"left","right","both","lower","upper"});
    addBuiltin(db, "z buffer",         C::Option, {"axis"}, {},
               {"auto","default","none","sort","reverse x","reverse y","reverse z"});
    addBuiltin(db, "surf",             C::Option, {"axis"});
    addBuiltin(db, "table",            C::Option, {"axis"});
    addBuiltin(db, "ybar interval",    C::Option, {"axis","groupplot"});
    addBuiltin(db, "xbar interval",    C::Option, {"axis","groupplot"});
    addBuiltin(db, "boxplot",          C::Option, {"axis"}, {}, {}, {"pgfplots.statistics"});

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

    // ── Callout shapes ── (verified in pgflibraryshapes.callouts.code.tex)
    const char *calloutShapes[] = {
        "rectangle callout","ellipse callout","cloud callout",nullptr};
    for (int i = 0; calloutShapes[i]; i++)
        addBuiltin(db, calloutShapes[i], C::Shape, {"tikzpicture","scope"}, {"node"},
                   {}, {"shapes.callouts"});

    // ── Symbol shapes ── (verified in pgflibraryshapes.symbols.code.tex)
    const char *symbolShapes[] = {
        "tape","magnetic tape","forbidden sign","correct forbidden sign",
        "signal","starburst","magnifying glass",nullptr};
    for (int i = 0; symbolShapes[i]; i++)
        addBuiltin(db, symbolShapes[i], C::Shape, {"tikzpicture","scope"}, {"node"},
                   {}, {"shapes.symbols"});

    // ── Geometric shapes (extras beyond tikz_keywords_basic.cpp) ──
    addBuiltin(db, "kite",          C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.geometric"});
    addBuiltin(db, "dart",          C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.geometric"});

    // ── Arrow shapes ──
    addBuiltin(db, "arrow box",     C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.arrows"});

    // ── Multipart shapes ──
    addBuiltin(db, "ellipse split", C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.multipart"});
    addBuiltin(db, "circle solidus", C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.multipart"});

    // ── Misc shapes ── (verified in pgflibraryshapes.misc.code.tex)
    addBuiltin(db, "cross out",     C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.misc"});
    addBuiltin(db, "strike out",    C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.misc"});
    addBuiltin(db, "chamfered rectangle", C::Shape, {"tikzpicture","scope"}, {"node"},
               {}, {"shapes.misc"});
    addBuiltin(db, "rounded rectangle", C::Shape, {"tikzpicture","scope"}, {"node"});
}

} // namespace TikzKeywords
