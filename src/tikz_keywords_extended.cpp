#include "tikz_keywords_data.h"

namespace TikzKeywords {

void registerExtended(Vec &db)
{
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
}

} // namespace TikzKeywords
