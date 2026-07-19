#include "tikz_keywords_data.h"

namespace TikzKeywords {

void registerExtended(Vec &db)
{
    // ── CircuiTikZ node shapes ──
    // Authoritative list of node shapes (logic ports, transistors, grounds,
    // supplies, amplifiers, terminals, ...) extracted from the CircuiTikZ 1.7.1
    // sources (\pgfdeclareshape, gate .style expansions and ground/supply
    // declarations). Registered as genuine shapes usable via node[...].
    // NOTE: Many bipoles also have a '<bipole>shape' form declared via
    // \pgfcircdeclarebipolescaled (pgfcirc.defines.tex:697); these are valid
    // node[...] shapes (see manual sections on bipole border anchors).
    auto ctikzNodeEnvs = {"tikzpicture","scope","circuitikz"};
    const char *ctikzNodeShapes[] = {
        "Lnigbt", "Lpigbt", "adder", "american and port", "american buffer port",
        "american nand port", "american nor port", "american not port", "american or port", "american xnor port",
        "american xor port", "and port", "antenna", "bare7seg", "bareRXantenna",
        "bareTXantenna", "bareantenna", "blockbox", "bnc", "box",
        "buffer", "buffer port", "cground", "circ", "circleinv",
        "circulator", "clockwedge", "diamondpole", "dinantenna", "dipchip",
        "double tgate", "dynode", "eground", "eground2", "elmech", "en amp",
        "european and port", "european buffer port", "european nand port", "european nor port", "european not port",
        "european or port", "european xnor port", "european xor port", "fd inst amp", "fd op amp",
        "flipflop", "genericsplitter", "gm amp", "ground", "harmonics",
        "hemt", "ieeestd and port", "ieeestd buffer port", "ieeestd nand port", "ieeestd nor port",
        "ieeestd not port", "ieeestd or port", "ieeestd xnor port", "ieeestd xor port",
        "ieee double tgate", "ieee tgate", "inst amp",
        "inst amp ra", "invschmitt", "invschmitt port", "jump crossing", "magnetron", "match",
        "mixer", "mslstub", "msport", "msrstub", "muxdemux",
        "mzm", "nand port", "nfet", "nground", "nigbt",
        "nigfetd", "nigfete", "nigfetebulk", "njfet", "nmos",
        "nmosd", "nor port", "not port", "notcirc", "npn",
        "ocirc", "odiamondpole", "op amp", "or port", "oscillator",
        "osquarepole", "pfet", "pground", "pigbt", "pigfetd",
        "pigfete", "pigfetebulk", "pjfet", "plain amp", "plain crossing",
        "plain mono amp", "pmos", "pmosd", "pnp", "potentiometershape", "proximeter",
        "qfpchip", "rground", "rotaryswitch", "rxantenna", "schmitt",
        "schmitt port", "schmitt symbol", "sground", "spdt", "splitter", "squarepole",
        "tgate", "tground", "tlground", "tlinestub", "txantenna", "vcc",
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

    // ── CircuiTikZ options ──
    // Audited against CircuiTikZ 1.7.1 sources (TeX Live 2025):
    //   pgfcirc.defines.tex, pgfcirccurrent.tex, pgfcircvoltage.tex,
    //   pgfcircflow.tex, pgfcirclabel.tex, pgfcircpath.tex, pgfcirctripoles.tex.
    // Entries are env-free but gated on the circuitikz lib, so they complete
    // both inside \begin{circuitikz} and in a tikzpicture using the package.
    auto ctkOpt = [&db](const char *name,
                        std::initializer_list<const char *> cmds = {},
                        std::initializer_list<const char *> vals = {}) {
        addBuiltin(db, name, C::Option, {}, cmds, vals, {"circuitikz"});
    };

    // \ctikzset configuration keys (also valid inside to[...] after a
    // component: the component style switches the key family to
    // /tikz/circuitikz, and \ctikzset{...} works anywhere).
    ctkOpt("bipoles/length");
    ctkOpt("bipoles/thickness");
    ctkOpt("thickness");
    ctkOpt("nodes width");
    ctkOpt("tripoles/mos style");
    ctkOpt("current/distance");
    // 1.x replaced the old voltage/distance by the from-node/from-line pair
    // (pgfcirc.defines.tex:1224/1229); "voltage/distance" and "current/shift"
    // do not exist and were previously offered by mistake.
    ctkOpt("voltage/distance from node");
    ctkOpt("voltage/distance from line");
    ctkOpt("flow/distance");
    ctkOpt("flow/offset");
    ctkOpt("label distance");
    ctkOpt("annotation distance");
    // Two-line label/annotation alignment (pgfcirclabel.tex:327/328/365/366).
    ctkOpt("l2 valign", {"draw","path","to"}, {"t","c","b"});
    ctkOpt("l2 halign", {"draw","path","to"}, {"l","c","r"});
    ctkOpt("a2 valign", {"draw","path","to"}, {"t","c","b"});
    ctkOpt("a2 halign", {"draw","path","to"}, {"l","c","r"});
    ctkOpt("label/align", {}, {"straight","rotate","smart"});
    ctkOpt("voltage dir", {}, {"old","noold","RP","EF"});
    ctkOpt("voltage", {}, {"american","european","straight","raised","curved"});
    ctkOpt("current", {}, {"american","european"});
    ctkOpt("american open voltage", {}, {"center","legacy"});
    ctkOpt("logic ports", {}, {"american","european","ieee"});
    ctkOpt("logic ports/scale");

    // Component-class styling matrix (pgfcirc.defines.tex:1010-1140 and the
    // class declarations across the shape files): <class>/fill, <class>/scale
    // and <class>/thickness exist for every component class. The reserved
    // pseudo-classes default/* and none/* are marked "do not touch" in the
    // sources and are deliberately omitted.
    for (const char *cls : {"amplifiers","batteries","blocks","capacitors",
                            "chips","connectors","csources","diodes",
                            "displays","electromechanicals","flipflops",
                            "grounds","inductors","instruments","logic ports",
                            "mechanicals","misc","muxdemuxes","power supplies",
                            "resistors","RF","sources","switches",
                            "transistors","tubes"}) {
        for (const char *sub : {"fill","scale","thickness"}) {
            const QByteArray key = QByteArray(cls) + '/' + sub;
            addBuiltin(db, key.constData(), C::Option, {}, {}, {},
                       {"circuitikz"});
        }
    }
    // Relative/modifier thickness fine-tuning keys (same source region).
    for (const char *k : {"resistors/modifier thickness",
                          "capacitors/modifier thickness",
                          "inductors/modifier thickness",
                          "transistors/modifier thickness",
                          "crossing vertical/relative thickness",
                          "opto arrows/relative thickness",
                          "switch arrows/relative thickness",
                          "transformer core/relative thickness",
                          "transistor bodydiode/relative thickness",
                          "transistor bodydiode/scale",
                          "transistor circle/relative thickness",
                          "tripoles/thickness","quadpoles/thickness",
                          "multipoles/thickness","seven seg/thickness"})
        ctkOpt(k);

    // Bipole annotation keys on to[...] components. Full variant sets from the
    // sources — current (pgfcirccurrent.tex): 13 variants; voltage
    // (pgfcircvoltage.tex): 9 variants (no v>^/v>_/v<^/v<_ exist); flow
    // (pgfcircflow.tex): 13 variants; label/annotation (pgfcirclabel.tex).
    for (const char *k : {"l","l^","l_","l2","l2^","l2_","l2 above","l2 below",
                          "label above","label below",
                          "a","a^","a_","a2","a2^","a2_","a2 above","a2 below",
                          "annotation","annotation above","annotation below",
                          "v","v^","v_","v<","v>","v^>","v^<","v_>","v_<",
                          "i","i^","i_","i<","i>","i^>","i^<","i_>","i_<",
                          "i>^","i>_","i<^","i<_",
                          "f","f^","f_","f<","f>","f^>","f^<","f_>","f_<",
                          "f>^","f>_","f<^","f<_",
                          "i symbols","no i symbols","v symbols","no v symbols",
                          "f symbols","no f symbols"})
        ctkOpt(k, {"draw","path","to"});
    // Path modifiers for orienting components (pgfcircpath.tex:34/41).
    ctkOpt("mirror", {"draw","path","to"});
    ctkOpt("invert", {"draw","path","to"});

    // Per-component modifier keys mirrored to /tikz/ by the sources
    // (\pgfkeys{/tikz/<key>/.add code=...} in pgfcirctripoles.tex,
    // pgfcircquadpoles.tex, pgfcircmonopoles.tex, pgfcirc.defines.tex, ...)
    // for direct use inside node[...] / to[...] — e.g.
    // node[op amp, noinv input up] or nmos[arrowmos]. Core TikZ keys that
    // circuitikz merely augments (color/fill/dashed) are not repeated here.
    for (const char *k : {// op amps (pgfcirctripoles.tex:6025-6028 ff)
                          "noinv input up","noinv input down",
                          "noinv output up","noinv output down",
                          // transistors
                          "arrowmos","noarrowmos","bodydiode","nobodydiode",
                          "bulk","nobulk","doublegate","nodoublegate",
                          "ferroel gate","no ferroel gate","nogate",
                          "schottky base","no schottky base","nobase",
                          "emptycircle","nocircle","fullcircle",
                          "bjt multi height","bjt pins width",
                          "collectors","emitters",
                          "center transistors text","legacy transistors text",
                          // diodes / tubes / nixie
                          "photo","filament","anodedot","nixieanode",
                          "fullcathode","nocathode",
                          // chips / mux-demux
                          "num pins","number inputs","hide numbers",
                          "show numbers","rotated numbers","straight numbers",
                          "external pins width","external pad fraction",
                          "no topmark","topmark",
                          // instruments / misc
                          "rotated instruments","straight instruments",
                          "nogrid","solderdot","nosolderdot",
                          "onelinechoke","twolineschoke","mstlinelen",
                          // amp/quadpole boxing (pgfcircquadpoles.tex:613-625)
                          "box","boxed","box only","boxed only",
                          // component text shortcuts (defines.tex:906-908,
                          // quadpoles.tex:527-535)
                          "t","t1","t2","text in","text out"})
        ctkOpt(k, {"node","draw","path","to"});

    // /tikz/-level styles (\tikzset in the sources) — these also work directly
    // in the environment options, e.g. \begin{circuitikz}[american, voltage
    // shift=1] (pgfcirc.defines.tex:1140/1141/1200/900, pgfcirccurrent.tex,
    // pgfcircvoltage.tex, pgfcircflow.tex, pgfcirclabel.tex, ...).
    for (const char *k : {"american","european",
                          "american currents","american voltages",
                          "american resistors","american inductors",
                          "american ports","european currents",
                          "european voltages","european resistors",
                          "european inductors","european ports",
                          "american gas filled surge arrester set",
                          "european gas filled surge arrester set",
                          "straight voltages","raised voltages",
                          "cute","cute inductors","ieee ports",
                          "voltage shift",
                          "bipole current style","bipole current append style",
                          "bipole voltage style","bipole voltage append style",
                          "bipole flow style","bipole flow append style",
                          "bipole label style","bipole label append style",
                          "bipole annotation style","bipole annotation append style",
                          "bipole nodes",
                          "full diodes","empty diodes","stroke diodes",
                          "full poles opacity","open poles opacity",
                          "all leads","no leads","input leads","output leads",
                          "no input leads","no output leads","reversed",
                          "amp plus","amp minus","amp symbol font",
                          "tr circle","tr gap fill",
                          "muxdemux def","muxdemux label","flipflop def",
                          "example circuit style","legacy circuit style",
                          "romano circuit style"})
        ctkOpt(k);
    ctkOpt("component text", {}, {"center","left"});

    // ── tikz-cd options ── (verified in tikzlibrarycd.code.tex)
    addBuiltin(db, "from",          C::Option, {"tikzcd"});
    addBuiltin(db, "to",            C::Option, {"tikzcd"});
    addBuiltin(db, "phantom",       C::Option, {"tikzcd"}, {"node"});
    addBuiltin(db, "description",   C::Option, {"tikzcd"});
    addBuiltin(db, "cramped",       C::Option, {"tikzcd"});
    addBuiltin(db, "shift left",    C::Option, {"tikzcd"});
    addBuiltin(db, "shift right",   C::Option, {"tikzcd"});
    addBuiltin(db, "shift",         C::Option, {"tikzcd"});
    addBuiltin(db, "crossing over", C::Option, {"tikzcd"});
    addBuiltin(db, "cells",         C::Option, {"tikzcd"});
    addBuiltin(db, "arrows",        C::Option, {"tikzcd"});
    addBuiltin(db, "labels",        C::Option, {"tikzcd"});
    addBuiltin(db, "marking",       C::Option, {"tikzcd"});
    addBuiltin(db, "math mode",     C::Option, {"tikzcd"});
    addBuiltin(db, "diagrams",      C::Option, {"tikzcd"});
    addBuiltin(db, "row sep",       C::Option, {"tikzcd"}, {},
               {"tiny","small","scriptsize","normal","large","huge"});
    addBuiltin(db, "column sep",    C::Option, {"tikzcd"}, {},
               {"tiny","small","scriptsize","normal","large","huge"});
    addBuiltin(db, "sep",           C::Option, {"tikzcd"}, {},
               {"tiny","small","scriptsize","normal","large","huge"});
    addBuiltin(db, "start anchor",  C::Option, {"tikzcd"});
    addBuiltin(db, "end anchor",    C::Option, {"tikzcd"});
    addBuiltin(db, "transform nodes", C::Option, {"tikzcd"});
    addBuiltin(db, "every arrow",   C::Option, {"tikzcd"});
    addBuiltin(db, "every label",   C::Option, {"tikzcd"});
    addBuiltin(db, "every diagram", C::Option, {"tikzcd"});
    addBuiltin(db, "every cell",    C::Option, {"tikzcd"});
    addBuiltin(db, "every matrix",  C::Option, {"tikzcd"});
    addBuiltin(db, "arrow style",   C::Option, {"tikzcd"},
               {"tikz","math","Latin Modern","Computer Modern"});
    // tikz-cd arrow-tip style keys (used inside \arrow[...])
    const char *cdArrowStyles[] = {
        "rightarrow","leftarrow","leftrightarrow","Rightarrow","Leftarrow",
        "Leftrightarrow","mapsto","mapsfrom","Mapsto","Mapsfrom",
        "hook","hook'","hookrightarrow","hookleftarrow",
        "harpoon","harpoon'","rightharpoonup","rightharpoondown",
        "leftharpoonup","leftharpoondown","tail","two heads","no head",
        "to head","no tail","maps to","dash","equal","equals",
        "dashed","dashrightarrow","dashleftarrow","squiggly",
        "rightsquigarrow","leftsquigarrow","leftrightsquigarrow",
        "rightarrowtail","leftarrowtail","twoheadrightarrow","twoheadleftarrow",
        "double line",
        nullptr };
    for (int i = 0; cdArrowStyles[i]; i++)
        addBuiltin(db, cdArrowStyles[i], C::Option, {"tikzcd"});

    // ── tikz-cd commands ── (\arrow / \ar and directional shorthands)
    const char *cdCmds[] = {
        "arrow","ar","rar","lar","uar","dar","urar","ular","drar","dlar",
        nullptr };
    for (int i = 0; cdCmds[i]; i++)
        addBuiltin(db, cdCmds[i], C::Command, {"tikzcd"});

    // ── graphs library ──
    // (\graph, the entry command, is already registered in
    // tikz_keywords_commands.cpp.)
    // Named subgraph structures (graphs.standard library). Verified against
    // tikzlibrarygraphs.standard.code.tex: subgraph I_n / I_nm / K_n / K_nm /
    // C_n / P_n / Grid_n / G_np. These are components used inside \graph{...}.
    for (const char *k : { "subgraph", "subgraph I_n", "subgraph I_nm",
                           "subgraph K_n", "subgraph K_nm", "subgraph C_n",
                           "subgraph P_n", "subgraph Grid_n", "subgraph G_np" })
        addBuiltin(db, k, C::Option, {}, {"draw","path"}, {},
                   {"graphs.standard"});
    // 'p' is the edge-probability key for the G_np random-graph model.
    addBuiltin(db, "p", C::Option, {}, {},
               {"0.5","0.25","0.75"}, {"graphs.standard"});

    // Simple/multi graph mode + the basic structure operators (verified in
    // tikzlibrarygraphs.code.tex). These apply to \graph, not a draw/path
    // command, so gate them by the 'graphs' library only.
    for (const char *k : { "simple", "multi", "clique", "cycle", "grid",
                           "no edges" })
        addBuiltin(db, k, C::Option, {}, {}, {}, {"graphs"});

    // ── graphs library: placement / growth / structure keys ──
    // (verified against tikzlibrarygraphs.code.tex). These are set on \graph[...]
    // (no draw/path command), so gate them by the 'graphs' library only. The
    // grow*/branch* keys take a distance; offer common distance hints.
    for (const char *k : { "grow right", "grow left", "grow up", "grow down",
                           "branch right", "branch left", "branch up", "branch down" })
        addBuiltin(db, k, C::Option, {}, {},
                   {"1cm","1.5cm","2cm","5mm","1em"}, {"graphs"});
    // The 'sep' variants default to 1em when given no value, but also accept
    // one; offer a small set of distance hints.
    for (const char *k : { "grow right sep", "grow left sep", "grow up sep",
                           "grow down sep", "branch right sep", "branch left sep",
                           "branch up sep", "branch down sep" })
        addBuiltin(db, k, C::Option, {}, {},
                   {"1em","5mm","1cm","2mm"}, {"graphs"});
    // Placement strategies and their parameters. ('radius', 'clique', 'cycle',
    // 'grid', 'nodes', 'name' are already registered above/elsewhere; 'radius'
    // is a general TikZ option so it is not re-registered here.)
    // NOTE: 'group count' is NOT a real key — the source only defines
    // 'chain count' and 'element count' (placement/.cd), so it is omitted.
    for (const char *k : { "Cartesian placement", "circular placement",
                           "grid placement", "no placement",
                           "chain shift", "group shift",
                           "chain polar shift", "group polar shift",
                           "clockwise", "counterclockwise", "phase",
                           "chain count", "element count",
                           "compute position", "place" })
        addBuiltin(db, k, C::Option, {}, {}, {}, {"graphs"});
    // Node / edge appearance and structure keys.
    for (const char *k : { "edges", "edge", "edge node", "edge label",
                           "edge label'", "edge quotes", "edge quotes center",
                           "edge quotes mid", "math nodes", "empty nodes",
                           "nodes", "number nodes", "number nodes sep", "typeset",
                           "name separator", "name", "as", "wrap after",
                           "operator", "set", "new set", "color class",
                           "left anchor", "right anchor", "trie",
                           "put node text on incoming edges",
                           "put node text on outgoing edges",
                           "source edge style", "source edge node", "source edge clear",
                           "target edge style", "target edge node", "target edge clear",
                           "clear >", "clear <" })
        addBuiltin(db, k, C::Option, {}, {}, {}, {"graphs"});
    // 'default edge kind' / 'default edge operator' accept the edge-kind
    // shorthands and structure operators respectively.
    addBuiltin(db, "default edge kind", C::Option, {}, {},
               {"--","->","<-","<->","-!-"}, {"graphs"});
    addBuiltin(db, "default edge operator", C::Option, {}, {},
               {"matching and star","complete bipartite","clique","cycle",
                "path","matching"}, {"graphs"});

    // Node-existence policy keys (verified: 'use existing nodes'/.is if and
    // 'fresh nodes'/.is if in tikzlibrarygraphs.code.tex). Boolean switches.
    for (const char *k : { "use existing nodes", "fresh nodes" })
        addBuiltin(db, k, C::Option, {}, {}, {}, {"graphs"});

    // The 'quick' switch enables the restricted fast-parsing syntax.
    addBuiltin(db, "quick", C::Option, {}, {}, {}, {"graphs"});

    // Edge-kind declarations and shorthands (new -> / new -- / ... and the
    // --, ->, <-, <->, -!- default-edge-kind shortcuts).
    for (const char *k : { "new ->", "new --", "new <-", "new <->", "new -!-" })
        addBuiltin(db, k, C::Option, {}, {}, {}, {"graphs"});

    // Node-set specification keys used with \graph[...]: V/W give explicit node
    // sets, n/m give counts, and name shore V/W name the two shores.
    for (const char *k : { "V", "W", "n", "m", "name shore V", "name shore W" })
        addBuiltin(db, k, C::Option, {}, {}, {}, {"graphs"});

    // 'declare' registers a new named subgraph macro.
    addBuiltin(db, "declare", C::Option, {}, {}, {}, {"graphs"});

    // Named-graph structure styles (component syntax used inside \graph{...}).
    for (const char *k : { "complete bipartite", "induced complete bipartite",
                           "induced independent set", "path", "induced path",
                           "induced cycle", "matching", "matching and star",
                           "butterfly", "butterfly'" })
        addBuiltin(db, k, C::Option, {}, {}, {}, {"graphs"});
    // 'every graph' style hook.
    addBuiltin(db, "every graph", C::Option, {"tikzpicture","scope"}, {}, {}, {"graphs"});

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

    // ═══════════════════════════════════════════════════════════════
    //  chemfig 1.66 (activated by \usepackage{chemfig})
    //  Commands + \setchemfig keys verified against
    //  texmf-dist/tex/generic/chemfig/chemfig.tex & chemfig-lewis.tex.
    // ═══════════════════════════════════════════════════════════════
    const char *chemfigCmds[] = {
        "chemfig","printatom","definesubmol","redefinesubmol","chemskipalign",
        "hflipnext","vflipnext","chemabove","Chemabove","chembelow","Chembelow",
        "chemname","chemnameinit","chemmove","chemleft","chemright","chemup",
        "chemdown","polymerdelim","Lewis","lewis","charge","Charge","chargedot",
        "chargeddot","chargerect","chargeline","setcharge","resetcharge",
        "enableshortcuts","disableshortcuts","schemestart","schemestop","arrow",
        "merge","subscheme","definearrow","setchemfig","resetchemfig",
        nullptr };
    for (int i = 0; chemfigCmds[i]; i++)
        addBuiltin(db, chemfigCmds[i], C::Command, {}, {}, {}, {"chemfig"});
    // \setchemfig{...} option keys
    const char *chemfigKeys[] = {
        "atom sep","bond offset","double bond sep","angle increment","node style",
        "bond style","atom style","chemfig style","cram width","cram dash width",
        "cram dash sep","cram rectangle","cycle radius coeff","stack sep",
        "compound style","compound sep","arrow offset","arrow angle","arrow coeff",
        "arrow style","arrow double sep","arrow double coeff","arrow label sep",
        "arrow head","bond join","fixed length","baseline","scheme debug","debug",
        "lewis sep","lewis length","lewis style","lewis dist","lewis radius",
        nullptr };
    for (int i = 0; chemfigKeys[i]; i++)
        addBuiltin(db, chemfigKeys[i], C::Option, {}, {}, {}, {"chemfig"});

    // ═══════════════════════════════════════════════════════════════
    //  tikz-feynman 1.1.0 (activated by \usepackage{tikz-feynman})
    //  Commands, environment and particle/edge styles verified against
    //  texmf-dist/tex/latex/tikz-feynman/tikzlibraryfeynman.code.tex &
    //  tikzfeynman.keys.code.tex.
    // ═══════════════════════════════════════════════════════════════
    const char *feynmanCmds[] = {
        "feynmandiagram","tikzfeynmanset","feynman","vertex","diagram","graph",
        nullptr };
    for (int i = 0; feynmanCmds[i]; i++)
        addBuiltin(db, feynmanCmds[i], C::Command, {}, {}, {}, {"tikz-feynman"});
    // particle / edge line styles (used in edge[...] / diagram)
    const char *feynmanStyles[] = {
        "fermion","anti fermion","majorana","anti majorana","photon","boson",
        "charged boson","anti charged boson","scalar","charged scalar",
        "anti charged scalar","ghost","gluon","plain",
        "dot","empty dot","square dot","crossed dot","blob","particle",
        "with arrow","with reversed arrow","insertion",
        "momentum","momentum'","reversed momentum","rmomentum","rmomentum'",
        "half left","half right","quarter left","quarter right",
        "small","medium","large","inline",
        "every vertex","every edge","every diagram","every feynman",
        "horizontal","vertical","layered layout","spring layout","tree layout",
        nullptr };
    for (int i = 0; feynmanStyles[i]; i++)
        addBuiltin(db, feynmanStyles[i], C::Option, {}, {}, {}, {"tikz-feynman"});
}

} // namespace TikzKeywords
