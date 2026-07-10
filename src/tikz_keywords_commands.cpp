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
        "adc", "afuse", "ageneric", "aGTOb", "agtobar",
        "allornothing", "allpass", "american controlled current source", "american controlled voltage source", "american current source",
        "american gas filled surge arrester", "american inductive sensor", "american inductor", "american voltage source", "ammeter",
        "amp", "asymmetric fuse", "baertty", "bandpass", "bandstop",
        "bare jumper", "barrier", "battery", "battery1", "battery2",
        "bgenerator", "biast", "biD", "bidirectionaldiode", "bmultiwire",
        "bulb", "buzzer", "C", "camera", "capacitive sensor",
        "capacitor", "cC", "cceI", "cceV", "ccgsw",
        "ccsw", "ceI", "ceV", "cgenerator", "cI",
        "cisource", "cisourceAM", "cisourceC", "cisourceEU", "cisourcesin",
        "closed double solder jumper", "closed jumper", "closed solder jumper", "closing normal closed switch", "closing normal open switch",
        "closing switch", "cncs", "cnos", "cogsw", "controlled current source",
        "controlled isource", "controlled isourcesin", "controlled sinusoidal current source", "controlled sinusoidal voltage source", "controlled voltage source",
        "controlled vsource", "controlled vsourcesin", "cosw", "cpe", "crossing",
        "csI", "cspst", "csV", "current source", "currtap",
        "curved capacitor", "cute choke", "cute closed switch", "cute closing switch", "cute european controlled current source",
        "cute european controlled voltage source", "cute european current source", "cute european voltage source", "cute inductive sensor", "cute inductor",
        "cute open switch", "cute opening switch", "cV", "cvsource", "cvsourceAM",
        "cvsourceC", "cvsourceEU", "cvsourcesin", "D", "dac",
        "damper", "dcisource", "dcvsource", "detector", "diode",
        "dsp", "eC", "ecapacitor", "ecsource", "elko",
        "empty controlled source", "esource", "european controlled current source", "european controlled voltage source", "european current source",
        "european gas filled surge arrester", "european inductive sensor", "european inductor", "european voltage source", "feC",
        "ferrocap", "fft", "fiber", "fullgeneric", "fuse",
        "generic", "GTO", "gto", "GTOb", "gtobar",
        "hemt", "highpass", "highpass2", "I", "iamp",
        "iec connector", "iecconn", "iloop", "iloop2", "inductive sensor",
        "inductor", "inerter", "ioosource", "isource", "isourceAM",
        "isourceC", "isourceEU", "isourceN", "isourcesin", "L",
        "lamp", "lasD", "laser diode", "ldR", "leD",
        "led", "left double solder jumper", "light dependent resistor", "Lnigbt", "loudspeaker",
        "lowpass", "lowpass2", "Lpigbt", "mass", "memristor",
        "mic", "mov", "Mr", "mstline", "multiwire",
        "ncpb", "ncpbo", "ncs", "neonlampac", "neonlampcc",
        "nfet", "ngenerator", "nI", "nigbt", "nigfetd",
        "nigfete", "nigfetebulk", "njfet", "nmos", "nmosd",
        "noise current source", "noise voltage source", "nopb", "nopbc", "norator",
        "normal closed switch", "normal open switch", "normally closed push button", "normally closed push button open", "normally open push button",
        "normally open push button closed", "nos", "npn", "nullator", "nV",
        "ohmmeter", "oncs", "onos", "ooosource", "oosourcetrans",
        "open", "open double solder jumper", "open jumper", "open solder jumper", "openbarrier",
        "opening normal closed switch", "opening normal open switch", "opening switch", "oscope", "ospst",
        "pC", "pD", "pfet", "phaseshifter", "photodiode",
        "photoresistor", "phR", "piattenuator", "piezoelectric", "pigbt",
        "pigfetd", "pigfete", "pigfetebulk", "pjfet", "pmos",
        "pmosd", "pnp", "polar capacitor", "potentiometer", "power",
        "pR", "push button", "PUT", "put", "pvmodule",
        "pvsource", "PZ", "qgenerator", "qiprobe", "qpprobe",
        "qvprobe", "R", "rbuzzer", "reed", "register",
        "relais", "resistive sensor", "resistor", "right double solder jumper", "rmeter",
        "rmeterwa", "sacac", "sacdc", "saturation", "sC",
        "Schottky diode", "sD", "sdcac", "sdcdc", "sgeneric",
        "shD", "Shockley diode", "short", "sI", "sigmoid",
        "sinetable", "sinusoidal current source", "sinusoidal voltage source", "sL", "smeter",
        "solar", "sparkgap", "spring", "spst", "square voltage source",
        "squid", "sqV", "sR", "sV", "switch",
        "swr", "tacac", "tacdc", "tattenuator", "tD",
        "tdcac", "tfullgeneric", "tgeneric", "thermistor", "thermistor ntc",
        "thermistor ptc", "thermocouple", "thR", "three-pins jumper", "thRn",
        "thRp", "thyristor", "TL", "tline", "tlmic",
        "tmultiwire", "toggle switch", "Tr", "transmission line", "triac",
        "triangle voltage source", "trx", "tunnel diode", "tV", "TVS diode",
        "tvsD", "tvset", "twoport", "twoportsplit", "Ty",
        "V", "vallpass", "vamp", "varcap", "variable american inductor",
        "variable capacitor", "variable cute inductor", "variable european inductor", "variable inductor", "variable resistor",
        "varistor", "VC", "vC", "vcc", "vco",
        "vdd", "vee", "viscoe", "vL", "voltage source",
        "voltmeter", "voosource", "vphaseshifter", "vpiattenuator", "vR",
        "vsource", "vsourceAM", "vsourceC", "vsourceEU", "vsourceN",
        "vsourcesin", "vsourcesquare", "vsourcetri", "vss", "vtattenuator",
        "wfuse", "wiggly fuse", "xgeneric", "xing", "zD",
        "Zener diode", "zzD", "ZZener diode",
        nullptr
    };
    for (int i = 0; ctikzComponents[i]; i++) {
        addBuiltin(db, ctikzComponents[i], C::Option, {}, {"draw","path","to"}, {}, {"circuitikz"});
    }

    // ═══════════════════════════════════════════════════════════════
    //  tkz-euclide v5.10c commands (activated by \usepackage{tkz-euclide})
    //  User-facing macros verified against texmf-dist/tex/latex/tkz-euclide/.
    //  Note: v5 has no \tkzDrawTriangle/\tkzDrawSquare/\tkzTangent/etc — one
    //  defines objects (\tkzDefTriangle/\tkzDefSquare) then draws with
    //  \tkzDrawPolygon; those bogus v4 names were removed.
    // ═══════════════════════════════════════════════════════════════
    auto tkeEnvs = {"tikzpicture","scope"};
    const char *tkeCmds[] = {
        // init / setup
        "tkzInit","tkzClip","tkzClipBB","tkzClipCircle","tkzClipPolygon",
        "tkzClipSector","tkzShowBB","tkzGrid","tkzHelpGrid","tkzText","tkzLegend",
        "tkzSetUpPoint","tkzSetUpLine","tkzSetUpLabel","tkzSetUpArc","tkzSetUpCircle",
        "tkzSetUpColors","tkzSetUpCompass","tkzSetUpAxis","tkzSetUpGrid",
        // define points
        "tkzDefPoint","tkzDefPoints","tkzDefMidPoint","tkzDefBarycentricPoint",
        "tkzDefShiftPoint","tkzDefShiftPointCoord","tkzDefPointBy","tkzDefPointsBy",
        "tkzDefPointWith","tkzDefPointOnLine","tkzDefPointOnCircle",
        "tkzDefRandPointOn","tkzDefEquiPoints","tkzDefProjExcenter",
        // define lines
        "tkzDefLine","tkzDefLineLL","tkzDefParallelogram","tkzDefRectangle",
        "tkzDefMediatorLine","tkzDefBisectorLine","tkzDefBisectorOutLine",
        "tkzDefOrthLine","tkzDefAltitudeLine","tkzDefSymmedianLine",
        "tkzDefTangent","tkzDefEulerLine",
        // define circles
        "tkzDefCircle","tkzDefCircleBy","tkzDefCircleR","tkzDefCircleD",
        "tkzDefInCircle","tkzDefExCircle","tkzDefCircumCircle","tkzDefEulerCircle",
        "tkzDefSpiekerCircle","tkzDefApolloniusCircle","tkzDefOrthogonalCircle",
        "tkzDefRadicalAxis","tkzDefInversionCircle","tkzDefMidArc",
        // define polygons / triangles
        "tkzDefSquare","tkzDefRectangle","tkzDefRegPolygon","tkzDefTriangle",
        "tkzDefTriangleCenter","tkzDefEquilateral","tkzDefEuclideTriangle",
        "tkzDefEulerTriangle","tkzDefExcentralTriangle","tkzDefIncentralTriangle",
        "tkzDefOrthicTriangle","tkzDefIntouchTriangle",
        "tkzDefExtouchTriangle","tkzDefTangentialTriangle","tkzDefGoldenTriangle",
        "tkzDefTwoAnglesTriangle","tkzDefPythagore","tkzDefGoldRectangle",
        // centers / getters
        "tkzGetPoint","tkzGetPoints","tkzGetFirstPoint","tkzGetSecondPoint",
        "tkzGetThirdPoint","tkzGetLength","tkzGetAngle","tkzGetPointCoord",
        "tkzCentroid","tkzCircumCenter","tkzInCenter","tkzExCenter","tkzOrthoCenter",
        "tkzEulerCenter","tkzRegPolygonCenter","tkzRenamePoint","tkzSwapPoints",
        // intersections
        "tkzInterLL","tkzInterLC","tkzInterCC","tkzInterLCR","tkzInterCCR",
        // draw points/lines/segments
        "tkzDrawPoint","tkzDrawPoints","tkzDrawLine","tkzDrawLines",
        "tkzDrawSegment","tkzDrawSegments","tkzDrawPolySeg","tkzDrawX","tkzDrawY",
        // draw polygons / circles / arcs / sectors
        "tkzDrawPolygon","tkzDrawPolygons","tkzDrawCircle","tkzDrawCircles",
        "tkzDrawSemiCircle","tkzDrawEllipse","tkzDrawArc","tkzDrawArcR",
        "tkzDrawArcAngles","tkzDrawSector","tkzDrawSectorR",
        // fill
        "tkzFillPolygon","tkzFillCircle","tkzFillCircles","tkzFillAngle",
        "tkzFillAngles","tkzFillSector",
        // angles
        "tkzLabelAngle","tkzLabelAngles","tkzMarkAngle",
        "tkzMarkAngles","tkzMarkRightAngle","tkzMarkRightAngles","tkzPicAngle",
        "tkzPicRightAngle","tkzFindAngle","tkzFindSlopeAngle",
        // labels / marks
        "tkzLabelPoint","tkzLabelPoints","tkzAutoLabelPoints","tkzLabelSegment",
        "tkzLabelSegments","tkzLabelLine","tkzLabelCircle","tkzLabelArc",
        "tkzLabelRegPolygon","tkzMarkSegment","tkzMarkSegments","tkzMarkArc",
        // transformations / compass / show
        "tkzCompass","tkzCompasss","tkzProtractor",
        "tkzShowLine","tkzShowLLLine","tkzShowMediatorLine","tkzShowProjection",
        "tkzShowTransformation","tkzCalcLength","tkzDuplicateSegment",
        "tkzProjection","tkzTranslation","tkzInversePoint","tkzInverseNegativePoint",
        "tkzRotateAngle","tkzURotateAngle",
        nullptr
    };
    for (int i = 0; tkeCmds[i]; i++)
        addBuiltin(db, tkeCmds[i], C::Command, tkeEnvs, {}, {}, {"tkz-euclide"});

    // value hints for a few key tkz-euclide commands
    addBuiltin(db, "tkzDefPointBy",    C::Command, tkeEnvs, {},
               {"translation","rotation","homothety","reflection","symmetry","projection","inversion"},
               {"tkz-euclide"});
    addBuiltin(db, "tkzDefLine",       C::Command, tkeEnvs, {},
               {"parallel","orthogonal","perpendicular","mediator","bisector","tangent"},
               {"tkz-euclide"});
    addBuiltin(db, "tkzDefCircle",     C::Command, tkeEnvs, {},
               {"through","R","diameter","in","circum","ex","apollonius","euler","orthogonal"},
               {"tkz-euclide"});
    addBuiltin(db, "tkzDefTriangleCenter", C::Command, tkeEnvs, {},
               {"centroid","in","circum","ortho","euler","nine","ex","median","symmedian","gergonne","nagel","spieker"},
               {"tkz-euclide"});

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
