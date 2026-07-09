#include "tikz_keywords_data.h"

namespace TikzKeywords {

void registerExtended(Vec &db)
{
    // ── CircuiTikZ node shapes ──
    // Authoritative list of node shapes (logic ports, transistors, grounds,
    // supplies, amplifiers, terminals, ...) extracted from the CircuiTikZ 1.7.1
    // sources (\pgfdeclareshape, gate .style expansions and ground/supply
    // declarations). Registered as genuine shapes usable via node[...] — no
    // bogus 'shape'-suffixed or misspelled entries are generated.
    auto ctikzNodeEnvs = {"tikzpicture","scope","circuitikz"};
    const char *ctikzNodeShapes[] = {
        "Lnigbt", "Lpigbt", "adder", "american and port", "american buffer port",
        "american nand port", "american nor port", "american not port", "american or port", "american xnor port",
        "american xor port", "and port", "antenna", "bare7seg", "bareRXantenna",
        "bareTXantenna", "bareantenna", "blockbox", "bnc", "box",
        "buffer", "buffer port", "cground", "circ", "circleinv",
        "circulator", "clockwedge", "diamondpole", "dinantenna", "dipchip",
        "dynode", "eground", "eground2", "elmech", "en amp",
        "european and port", "european buffer port", "european nand port", "european nor port", "european not port",
        "european or port", "european xnor port", "european xor port", "fd inst amp", "fd op amp",
        "flipflop", "genericsplitter", "gm amp", "ground", "harmonics",
        "hemt", "ieeestd and port", "ieeestd buffer port", "ieeestd nand port", "ieeestd nor port",
        "ieeestd not port", "ieeestd or port", "ieeestd xnor port", "ieeestd xor port", "inst amp",
        "inst amp ra", "invschmitt", "jump crossing", "magnetron", "match",
        "mixer", "mslstub", "msport", "msrstub", "muxdemux",
        "mzm", "nand port", "nfet", "nground", "nigbt",
        "nigfetd", "nigfete", "nigfetebulk", "njfet", "nmos",
        "nmosd", "nor port", "not port", "notcirc", "npn",
        "ocirc", "odiamondpole", "op amp", "or port", "oscillator",
        "osquarepole", "pfet", "pground", "pigbt", "pigfetd",
        "pigfete", "pigfetebulk", "pjfet", "plain amp", "plain crossing",
        "plain mono amp", "pmos", "pmosd", "pnp", "proximeter",
        "qfpchip", "rground", "rotaryswitch", "rxantenna", "schmitt",
        "schmitt symbol", "sground", "spdt", "splitter", "squarepole",
        "tground", "tlground", "tlinestub", "txantenna", "vcc",
        "vdd", "vee", "vss", "waves", "wedgeinv",
        "wilkinson", "xnor port", "xor port",
        nullptr
    };
    for (int i = 0; ctikzNodeShapes[i]; i++) {
        addBuiltin(db, ctikzNodeShapes[i], C::Shape, ctikzNodeEnvs,
                   {"node","path","draw","to"}, {}, {"circuitikz"});
    }

    // ── Decorations ──
    // Canonical decoration names/libraries are registered in
    // registerDecorations() (tikz_keywords_basic.cpp). Only the extra
    // "footprints of" key (a decoration option, not a decoration name) is kept
    // here; it is verified in decorations.footprints.
    addBuiltin(db, "footprints of",   C::Option, {}, {"draw","path"},
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
    addBuiltin(db, "legend image post style",C::Option, {"axis","groupplot"});

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
    addBuiltin(db, "area style",C::Option, {"axis","groupplot"});
    addBuiltin(db, "hist",      C::Option, {"axis","groupplot"}, {}, {}, {"pgfplots.statistics"});

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
    addBuiltin(db, "variable y", C::Option, {"axis","groupplot"});
    addBuiltin(db, "parametric", C::Option, {"axis","groupplot"});

    // ── pgfplots const plot variants ──
    // (const plot mark left/right/mid are core TikZ plot handlers registered in
    //  tikz_keywords_options.cpp; they also work inside pgfplots axes.)

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
    addBuiltin(db, "a_",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "a^",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "v",   C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "v_",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "v^",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "i",   C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "i_",  C::Option, {"circuitikz"}, {"draw","path"});
    addBuiltin(db, "i^",  C::Option, {"circuitikz"}, {"draw","path"});
    // Path modifiers for orienting/labelling components (\ctikzset mirror/.style,
    // invert/.style; verified against CircuiTikZ 1.7.1 sources).
    addBuiltin(db, "mirror", C::Option, {"circuitikz"}, {"draw","path","to"});
    addBuiltin(db, "invert", C::Option, {"circuitikz"}, {"draw","path","to"});

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

    // ── arrows.meta arrow tips ── (verified in pgflibraryarrows.meta.code.tex)
    // These modern tips and aliases require \usetikzlibrary{arrows.meta}.
    const char *metaArrows[] = {
        "Stealth","Latex","Kite","Rays","Tee Barb","Bar",
        "Straight Barb","Arc Barb","Hooks","Circle","Square","Diamond",
        "Turned Square","Triangle","Rectangle","Ellipse",
        "LaTeX","To","Bracket","Parenthesis","Implies",
        "Fast Triangle","Fast Round","Round Cap","Butt Cap","Triangle Cap",
        "Computer Modern Rightarrow","Classical TikZ Rightarrow",
        nullptr};
    for (int i = 0; metaArrows[i]; i++)
        addBuiltin(db, metaArrows[i], C::Arrow, {}, {"draw","path"}, {}, {"arrows.meta"});

    // ── Additional libraries ──
    addBuiltin(db, "shadows.blur",         C::Library);
    addBuiltin(db, "graphs.simple",        C::Library);
    addBuiltin(db, "pgfplots.fillbetween", C::Library);

    // ── Additional commands ──
    // Only genuine \backslash commands belong here. Parameter-type keywords
    // ("table"/"coordinates"/"graphics"), path operations ("plot") and syntax
    // fragments ("at") are NOT standalone commands, and mis-cased/duplicate
    // entries were removed.
    addBuiltin(db, "addplot+",           C::Command, {"axis","groupplot"});
    addBuiltin(db, "pgfplotstableset",   C::Command);
    addBuiltin(db, "tikzmath",           C::Command);
}

} // namespace TikzKeywords
