#include "tikz_keywords_data.h"

namespace TikzKeywords {

void registerCommands(Vec &db)
{
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
        "begin",
        "caption","clip","closedcycle","coordinate",
        "cos",
        "datavisualization",
        "definecolor","def","draw",
        "edef","else","end","exp","expandafter",
        "fill","filldraw","flat","foreach",
        "gdef","global","graph",
        "hbox","hfil","hfill","hline","hrule","hskip","hspace",
        "if","ifdim","ifnum","ifodd","ifx",
        "input","item",
        "label","let","line","ln",
        "matrix","max","min","moveleft","moveright",
        "newcommand","newcount","newdimen","newif","newwrite",
        "node",
        "par","parbox","path","pgfkeys","pgfmathsetmacro","pgfmathsetlength",
        "pgfplotstableread","pgfplotstabletypeset",
        "pic","protected",
        "relax","renewcommand","repeat",
        "setlength","shade","shadedraw",
        "sin","sqrt",
        "text","tikzset","tikzstyle","to",
        "usebox","usepackage","usetikzlibrary",
        "vbox","vfil","vfill","vrule","vskip","vspace",
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
        "pgfkeysvalueof","pgfkeysgetvalue",
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
        nullptr};
    for (int i = 0; cmds[i]; i++)
        addBuiltin(db, cmds[i], C::Command);

    // Path operations: completable as bare words inside a path (e.g. "arc",
    // "plot", "edge") but NOT valid as \backslash commands, so they live under
    // PathOperation rather than Command (which would offer a bogus "\arc").
    const char *pathOps[] = { "arc", "plot", "edge", nullptr };
    for (int i = 0; pathOps[i]; i++)
        addBuiltin(db, pathOps[i], C::PathOperation);
}

void registerMathFunctions(Vec &db)
{
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
}

} // namespace TikzKeywords
