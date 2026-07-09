#include "tikz_keywords_data.h"

namespace TikzKeywords {

void registerCommands(Vec &db)
{
    // ── Commands ── (LaTeX/TikZ/PGF commands)
    // ── CircuiTikZ path components (as Options, not Commands) ──
    // These are used in to[...] or node[...] as keys, not as \ backslash commands.
    // Filtering is done via requiredLibs = {"circuitikz"}.
    // Authoritative list of CircuiTikZ path components (bipole keys, full names
    // and style shortcuts) usable inside to[...] / node[...]. Extracted verbatim
    // from the CircuiTikZ 1.7.1 sources (\pgfcirc@activate@bipole* and
    // \pgfcirc@style@to@style), so no bogus/misspelled entries are offered.
    const char *ctikzComponents[] = {
        "C", "D", "GTO", "GTOb", "I",
        "L", "Lnigbt", "Lpigbt", "Mr", "PUT",
        "PZ", "R", "Schottky diode", "Shockley diode", "TL",
        "TVS diode", "Tr", "Ty", "V", "VC",
        "ZZener diode", "Zener diode", "aGTOb", "adc", "afuse",
        "ageneric", "agtobar", "allornothing", "allpass", "american controlled current source",
        "american controlled voltage source", "american current source", "american gas filled surge arrester", "american inductive sensor", "american inductor",
        "american voltage source", "americangfsurgearrester", "americaninductivesens", "americaninductor", "ammeter",
        "amp", "asymmetric fuse", "baertty", "bandpass", "bandstop",
        "bare jumper", "barrier", "battery", "battery1", "battery2",
        "batteryone", "batterytwo", "bgenerator", "biD", "biast",
        "bidirectionaldiode", "bjumper", "bmultiwire", "bulb", "buzzer",
        "cC", "cI", "cV", "camera", "capacitive sensor",
        "capacitivesens", "capacitor", "ccapacitor", "cceI", "cceV",
        "ccgsw", "ccsw", "cdsjumper", "ceI", "ceV",
        "cgenerator", "cisource", "cisourceAM", "cisourceC", "cisourceEU",
        "cisourceam", "cisourcesin", "cjumper", "closed double solder jumper", "closed jumper",
        "closed solder jumper", "closing normal closed switch", "closing normal open switch", "closing switch", "cncs",
        "cnos", "cogsw", "controlled current source", "controlled isource", "controlled isourcesin",
        "controlled sinusoidal current source", "controlled sinusoidal voltage source", "controlled voltage source", "controlled vsource", "controlled vsourcesin",
        "cosw", "cpe", "crossing", "csI", "csV",
        "csjumper", "cspst", "current source", "currtap", "curved capacitor",
        "cute choke", "cute closed switch", "cute closing switch", "cute european controlled current source", "cute european controlled voltage source",
        "cute european current source", "cute european voltage source", "cute inductive sensor", "cute inductor", "cute open switch",
        "cute opening switch", "cutechoke", "cuteclosedswitch", "cuteclosingswitch", "cuteinductivesens",
        "cuteinductor", "cuteopeningswitch", "cuteopenswitch", "cvsource", "cvsourceAM",
        "cvsourceC", "cvsourceEU", "cvsourceam", "cvsourcesin", "dac",
        "damper", "dcisource", "dcvsource", "detector", "diode",
        "dsp", "eC", "ecapacitor", "ecsource", "elko",
        "empty controlled source", "esource", "european controlled current source", "european controlled voltage source", "european current source",
        "european gas filled surge arrester", "european inductive sensor", "european inductor", "european voltage source", "europeangfsurgearrester",
        "europeaninductivesens", "europeaninductor", "feC", "ferrocap", "fft",
        "fiber", "fullgeneric", "fuse", "generic", "gto",
        "gtobar", "hemt", "highpass", "highpass2", "iamp",
        "iec connector", "iecconn", "iloop", "iloop2", "ilooptwo",
        "inerter", "ioosource", "isource", "isourceAM", "isourceC",
        "isourceEU", "isourceN", "isourceam", "isourcesin", "lamp",
        "lasD", "laser diode", "ldR", "ldsjumper", "leD",
        "led", "left double solder jumper", "loudspeaker", "lowpass", "lowpass2",
        "mass", "memristor", "mic", "mov", "mstline",
        "multiwire", "nI", "nV", "ncpb", "ncpbo",
        "ncpushbutton", "ncpushbuttono", "ncs", "neonlampac", "neonlampcc",
        "nfet", "ngenerator", "nigbt", "nigfetd", "nigfete",
        "nigfetebulk", "njfet", "nmos", "nmosd", "noise current source",
        "noise voltage source", "nopb", "nopbc", "norator", "normal closed switch",
        "normal open switch", "normally closed push button", "normally closed push button open", "normally open push button", "normally open push button closed",
        "nos", "npn", "nullator", "odsjumper", "ohmmeter",
        "ojumper", "oncs", "onos", "ooosource", "oosource",
        "oosourcetrans", "open", "open double solder jumper", "open jumper", "open solder jumper",
        "openbarrier", "opening normal closed switch", "opening normal open switch", "opening switch", "oscope",
        "osjumper", "ospst", "pC", "pD", "pR",
        "pfet", "phR", "phaseshifter", "photodiode", "photoresistor",
        "piattenuator", "piezoelectric", "pigbt", "pigfetd", "pigfete",
        "pigfetebulk", "pjfet", "pmos", "pmosd", "pnp",
        "polar capacitor", "polarcapacitor", "power", "push button", "pushbutton",
        "pushbuttonc", "put", "pvmodule", "pvsource", "qgenerator",
        "qiprobe", "qpprobe", "qvprobe", "rbuzzer", "rdsjumper",
        "reed", "register", "relais", "right double solder jumper", "rmeter",
        "rmeterwa", "sC", "sD", "sI", "sL",
        "sR", "sV", "sacac", "sacdc", "saturation",
        "sdcac", "sdcdc", "sgeneric", "shD", "short",
        "sigmoid", "sinetable", "sinusoidal current source", "sinusoidal voltage source", "smeter",
        "solar", "solarsource", "sparkgap", "spring", "spst",
        "sqV", "square voltage source", "squid", "switch", "swr",
        "tD", "tV", "tacac", "tacdc", "tattenuator",
        "tdcac", "tfullgeneric", "tgeneric", "thR", "thRn",
        "thRp", "thermistor", "thermistor ntc", "thermistor ptc", "thermistorntc",
        "thermistorptc", "thermocouple", "three-pins jumper", "thyristor", "tjumper",
        "tline", "tlmic", "tmultiwire", "toggle switch", "toggleswitch",
        "transmission line", "triac", "triangle voltage source", "trx", "tunnel diode",
        "tvsD", "tvset", "twoport", "twoportsplit", "vC",
        "vL", "vR", "vallpass", "vamericaninductor", "vamp",
        "varcap", "variable american inductor", "variable capacitor", "variable cute inductor", "variable european inductor",
        "varistor", "vcapacitor", "vco", "vcuteinductor", "vdd",
        "veuropeaninductor", "viscoe", "voltage source", "voltmeter", "voosource",
        "vphaseshifter", "vpiattenuator", "vsource", "vsourceAM", "vsourceC",
        "vsourceEU", "vsourceN", "vsourceam", "vsourcesin", "vsourcesquare",
        "vsourcetri", "vss", "vtattenuator", "wfuse", "wiggly fuse",
        "xgeneric", "xing", "zD", "zzD",
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

    // ═══════════════════════════════════════════════════════════════
    //  physics package commands (heavily used in physics diagrams)
    //  Activated by \usepackage{physics}. Names verified against
    //  texmf-dist/tex/latex/physics/physics.sty. Pure trig/log overlaps
    //  (\sin,\cos,\exp,\ln,\log,\tan,\det,\Re,\Im,\Pr,\abs) are already in
    //  the general math command list, so only physics-distinctive macros are
    //  registered here to avoid duplicate suggestions.
    const char *physicsCmds[] = {
        "va","vb","vu","vdot","vnabla","vectorbold","vectorarrow","vectorunit",
        "grad","gradient","divergence","curl","laplacian",
        "dd","dv","derivative","pdv","pderivative","partialderivative",
        "fdv","fderivative","functionalderivative","differential","flatfrac",
        "cross","cp","crossproduct","dotproduct","dyad","outerproduct",
        "innerproduct","ip","expval","expectationvalue","ev","var","variation",
        "bra","ket","braket","ketbra","mel","matrixel","matrixelement",
        "comm","commutator","acomm","anticommutator","pb","poissonbracket",
        "norm","absolutevalue","order","eval","evaluated",
        "qty","quantity","pqty","bqty","Bqty","vqty",
        "mqty","pmat","smqty","spmqty","Pmqty","sPmqty","bmqty","sbmqty",
        "vmqty","svmqty","pmqty",
        "matrixquantity","dmat","diagonalmatrix","admat","antidiagonalmatrix",
        "imat","identitymatrix","xmat","xmatrix","zmat","zeromatrix",
        "paulimatrix","paulixmatrix","pauliymatrix","paulizmatrix",
        "mdet","matrixdeterminant","smdet","tr","Tr",
        "Res","principalvalue","pv","PV",
        "qq","qqtext","qc","qcc","qcomma","qand","qor","qif","qthen",
        "qelse","qotherwise","qsince","qusing","qassume","qgiven","qlet",
        "qfor","qin","qunless",
        "coth","sech","csch",
        "arccot","arcsec","arccsc","acot","asec","acsc",
        nullptr
    };
    for (int i = 0; physicsCmds[i]; i++)
        addBuiltin(db, physicsCmds[i], C::Command, {}, {}, {}, {"physics"});

    // ═══════════════════════════════════════════════════════════════
    //  siunitx package commands. Activated by \usepackage{siunitx}.
    //  Public commands verified against texmf-dist/tex/latex/siunitx/siunitx.sty.
    const char *siunitxCmds[] = {
        "SI","si","num","unit","qty","ang",
        "numlist","numrange","numproduct","qtylist","qtyrange","qtyproduct",
        "SIlist","SIrange","sisetup","tablenum",
        "complexnum","complexqty","DeclareSIUnit",
        nullptr
    };
    for (int i = 0; siunitxCmds[i]; i++)
        addBuiltin(db, siunitxCmds[i], C::Command, {}, {}, {}, {"siunitx"});

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
    // The complete PGF math-engine function set, verified against
    // texmf-dist/tex/generic/pgf/math/ (\pgfmathdeclarefunction). Previous
    // bogus entries (cit, kil, note, record, res, vecilen, cm, pt, clip, in,
    // turn, format, intersection, dotproduct, power) were not real functions.
    const char *mathFns[] = {
        "abs","acos","add","and","approxequalto","array","asin","atan","atan2",
        "bin","ceil","cos","cosec","cosh","cot","deg","depth","dim",
        "div","divide","e","equal","exp","factorial","false","floor","frac",
        "gcd","greater","height","hex","Hex","ifthenelse","int","iseven",
        "isodd","isprime","less","ln","log10","log2","max","min","mod","Mod",
        "multiply","neg","not","notequal","notgreater","notless","oct","or",
        "pi","pow","rad","radians","rand","random","real","reciprocal","rnd",
        "round","scalar","sec","sign","sin","sinh","sqrt","subtract","tan",
        "tanh","true","veclen","width",
        nullptr};
    for (int i = 0; mathFns[i]; i++)
        addBuiltin(db, mathFns[i], C::MathFunction);
}

} // namespace TikzKeywords
