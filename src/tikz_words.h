#pragma once
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QSet>

namespace TikzWords {

inline const QStringList tikzCommands() {
    static const QStringList list = {
        // ── Basic TikZ/PGF ──
        "addplot", "addplot3", "addlegendentry", "addlegendimage",
        "anchor", "arc", "arrow",
        "ball", "begin", "bend",
        "caption", "circle", "clip", "closedcycle", "coordinate",
        "coordinates", "cos", "cycle",
        "dataset", "datavisualization",
        "definecolor", "def", "draw", "drawplot",
        "edef", "edge", "else", "end", "exp", "expandafter",
        "fill", "filldraw", "flat", "foreach", "forest",
        "getref", "gdef", "global", "graph",
        "hbox", "hfil", "hfill", "hline", "hrule", "hskip", "hspace",
        "if", "ifdim", "ifnum", "ifodd", "ifx",
        "input", "item",
        "label", "let", "line", "lineto", "ln",
        "matrix", "max", "min", "moveleft", "moveright",
        "newcommand", "newcount", "newdimen", "newif", "newwrite",
        "node",
        "par", "parbox", "path", "pattern", "pgfkeys", "pgfmathsetmacro",
        "pgfplotstableread", "pgfplotstabletypeset",
        "pic", "pin", "plot", "protected",
        "relax", "renewcommand", "repeat",
        "scope", "setlength", "shade", "shadedraw",
        "sin", "sqrt",
        "tabular", "text", "tikzset", "tikzstyle", "to",
        "usebox", "usepackage", "usetikzlibrary",
        "vbox", "vertex", "vfil", "vfill", "vrule", "vskip", "vspace",
        "xdef",

        // ── tikz-3dplot (verified from official v1.0 manual) ──
        "tdplotsetmaincoords",
        "tdplotsetrotatedcoords",
        "tdplotsetrotatedcoordsorigin",
        "tdplotresetrotatedcoordsorigin",
        "tdplotsetcoord",
        "tdplotsetthetaplanecoords",
        "tdplotsetrotatedthetaplanecoords",
        "tdplotsetpolarplotrange",
        "tdplotresetpolarplotrange",
        "tdplotdrawarc",
        "tdplotdrawpolytopearc",
        "tdplotsphericalsurfaceplot",
        "tdplotshowargcolorguide",
        "tdplotdefinepoints",
        "tdplotcalctransformmainrot",
        "tdplotcalctransformrotmain",
        "tdplotcalctransformmainscreen",
        "tdplottransformmainrot",
        "tdplottransformrotmain",
        "tdplottransformmainscreen",
        "tdplotgetpolarcoords",
        "tdplotcrossprod",
        "tdplotsinandcos",
        "tdplotmult",
        "tdplotdiv",

        // ── PGF Math / Keys ──
        "pgfmathtruncatemacro",
        "pgfmathparse",
        "pgfmathresult",
        "pgfmathprintnumber",
        "pgfmathrandominteger",
        "pgfmathsetseed",
        "pgfmathsetmacroglobal",
        "pgfkeysvalueof",
        "pgfkeysgetvalue",
        "pgfkeyssetvalue",
        "pgfplotsset",

        // ── PGF Layer Commands ──
        "pgfdeclarelayer",
        "pgfsetlayers",
        "pgfonlayer",
        "endpgfonlayer",

        // ── PGF Declarations ──
        "pgfdeclareshape",
        "pgfdeclareplotmark",
        "pgfdeclarepattern",
        "pgfdeclaredecoration",
        "pgfdeclarehorizontalshading",
        "pgfdeclareverticalshading",

        // ── PGF Drawing Primitives ──
        "pgfsetlinewidth",
        "pgfsetcolor",
        "pgfsetarrow",
        "pgfsetdash",
        "pgfsetbuttcap",
        "pgfsetroundcap",
        "pgfsetrectcap",
        "pgfsetroundjoin",
        "pgfsetbeveljoin",
        "pgfsetmiterjoin",
        "pgftext",
        "pgfnode",
        "pgfcoordinate",
        "pgfpathmoveto",
        "pgfpathlineto",
        "pgfpathcurveto",
        "pgfpathcircle",
        "pgfpathellipse",
        "pgfpathrectangle",
        "pgfpathclose",
        "pgfusepath",
        "pgfuseplotmark",
        "pgfpoint",
        "pgfpointanchor",

        // ── PGF Transformations ──
        "pgftransformshift",
        "pgftransformrotate",
        "pgftransformscale",
        "pgftransformxscale",
        "pgftransformyscale",
        "pgftransformxslant",
        "pgftransformyslant",
        "pgftransformcm",
        "pgftransformreset",

        // ── More TikZ drawing commands ──
        "useasboundingbox",
        "tikztonodes",
        "tikzlastnode",
        "tikzmark",
        "legend",

        // ── Math symbols (Greek, standard LaTeX) ──
        "alpha", "beta", "gamma", "delta", "epsilon", "varepsilon",
        "zeta", "eta", "theta", "vartheta", "iota", "kappa",
        "lambda", "mu", "nu", "xi", "pi", "varpi", "rho", "varrho",
        "sigma", "varsigma", "tau", "upsilon", "phi", "varphi",
        "chi", "psi", "omega",
        "Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma",
        "Upsilon", "Phi", "Psi", "Omega",

        // ── Standard LaTeX math operators ──
        "arccos", "arcsin", "arctan", "arg",
        "cot", "coth", "csc",
        "deg", "det", "dim", "hom", "ker",
        "lg", "lim", "liminf", "limsup", "sup", "inf",
        "max", "min",
        "gcd", "Pr",
        "sec", "sinh", "cosh", "tanh",
        "log",

        // ── Math delimiters ──
        "left", "right",
        "bigl", "Bigl", "biggl", "Biggl",
        "bigr", "Bigr", "biggr", "Biggr",
        "langle", "rangle",
        "lceil", "rceil", "lfloor", "rfloor",
        "lvert", "rvert", "lVert", "rVert",

        // ── Math accents ──
        "hat", "check", "tilde", "acute", "grave", "dot", "ddot",
        "breve", "bar", "vec",
        "widehat", "widetilde",
        "overline", "underline",
        "overrightarrow", "overleftarrow",
        "overbrace", "underbrace",

        // ── Math fonts / styles (standard) ──
        "mathbb", "mathbf", "mathrm", "mathsf", "mathtt",
        "mathit", "mathcal", "mathscr", "mathfrak",
        "text", "textbf", "textit", "texttt", "textsf", "textsc",

        // ── Math misc ──
        "frac", "dfrac", "tfrac", "binom", "dbinom", "tbinom",
        "int", "iint", "iiint", "oint",
        "sum", "prod", "coprod",
        "bigcup", "bigcap", "bigvee", "bigwedge",
        "bigoplus", "bigotimes", "bigodot", "biguplus", "bigsqcup",
        "partial", "nabla",
        "forall", "exists", "nexists", "emptyset", "varnothing",
        "infty", "aleph", "hbar",
        "imath", "jmath", "ell", "wp", "Re", "Im",
        "angle", "measuredangle",
        "to", "longrightarrow", "longleftarrow", "longleftrightarrow",
        "rightarrow", "leftarrow", "leftrightarrow",
        "mapsto", "longmapsto",
        "Rightarrow", "Leftarrow", "Leftrightarrow",
        "Longrightarrow", "Longleftarrow", "Longleftrightarrow",
        "approx", "sim", "simeq", "cong", "equiv",
        "propto", "neq", "pm", "mp",
        "times", "div", "cdot", "ast", "star",
        "otimes", "oplus", "ominus", "odot", "oslash",
        "cap", "cup", "setminus", "subset", "supset",
        "in", "ni", "notin",
        "leq", "geq", "ll", "gg",
        "parallel", "perp",
        "lll", "ggg",
        "subseteq", "supseteq", "subsetneq", "supsetneq",
        "smile", "frown", "bowtie",
        "ldots", "cdots", "vdots", "ddots",
        "xleftarrow", "xrightarrow",
        "displaystyle", "textstyle", "scriptstyle", "scriptscriptstyle",

        // ── Spacing ──
        "quad", "qquad", "hspace", "vspace",

        // ── Boxes / Layout ──
        "mbox", "makebox", "framebox",
        "parbox", "minipage",
        "noindent", "centering", "raggedright", "raggedleft",

        // ── More TeX basics ──
        "title", "author", "date", "maketitle",
        "chapter", "section", "subsection", "subsubsection",
        "emph",
        "url", "href",
        "includegraphics",
        "color", "textcolor", "colorbox", "fcolorbox",

        // ── CircuitikZ bipoles (verified from pgfcircbipoles.tex) ──
        "resistor", "vresistor",
        "potentiometer",
        "thermistor", "thermistorptc", "thermistorntc",
        "varistor", "mov", "memristor",
        "photoresistor",
        "generic", "ageneric", "tgeneric",
        "short", "open",
        "capacitor", "ecapacitor", "ccapacitor", "vcapacitor",
        "ferrocap", "piezoelectric", "cpe",
        "inductor",
        "battery", "battery1", "battery2",
        "lamp", "bulb",
        "fuse", "relais",
        "squid", "barrier",
        "thermocouple",
        "loudspeaker", "mic", "buzzer",
        "ammeter", "voltmeter", "ohmmeter", "oscope",
        "loudspeaker",
    };
    return list;
}

inline const QStringList tikzEnvironments() {
    static const QStringList list = {
        "tikzpicture", "scope", "pgfonlayer",
        "axis", "semilogxaxis", "semilogyaxis", "loglogaxis",
        "polaraxis", "smithchart", "ternaryaxis",
        "groupplot",
        "datavisualization",
        "tikzcd",
        "forest",
        "mindmap",
        "feynman",
        "circuitikz",
        "filecontents", "filecontents*",
        "comment",
        "document",
        "center", "flushleft", "flushright",
        "itemize", "enumerate", "description",
        "figure", "table", "minipage",
        "align", "align*",
        "equation", "equation*",
        "displaymath", "math",
        "abstract", "proof", "theorem", "lemma", "corollary", "definition",
        "array",
        "matrix", "pmatrix", "bmatrix", "Bmatrix", "vmatrix", "Vmatrix",
        "cases", "split", "gathered", "aligned", "alignedat",
        "gather", "gather*", "multline", "multline*",
        "tabularx", "longtable", "tabular",
        "wrapfigure",
        "tcolorbox",
        "overpic",
        "quote", "quotation", "verse",
        "appendix",
        "frame",
    };
    return list;
}

inline const QStringList tikzOptions() {
    static const QStringList list = {
        // ── Basic positioning ──
        "above", "above left", "above right",
        "below", "below left", "below right",
        "left", "right",
        "align", "anchor", "angle", "append after command",
        "arrow",
        "ball color", "baseline",
        "bend angle", "bend at end", "bend at start",
        "bend left", "bend pos", "bend right",
        "cap",
        "circle",
        "color", "column sep",
        "dash", "dash phase", "dashed",
        "decorate", "decoration",
        "diamond", "domain", "dotted",
        "double", "double distance", "double equal sign distance",
        "draw", "draw opacity", "drop shadow",
        "ellipse",
        "every edge", "every loop", "every node",
        "fill", "fill opacity",
        "font", "fontsize",
        "grow", "grow cyclic",
        "in", "out", "in looseness", "out looseness",
        "inner color", "inner sep", "inner xsep", "inner ysep",
        "join", "join round",
        "label",
        "late options", "left color", "left delimiter", "left=of",
        "level distance", "line cap", "line join", "line width",
        "looseness",
        "loosely dashed", "densely dashed",
        "loosely dotted", "densely dotted",
        "dash dot", "dash dot dot",
        "loosely dash dot", "densely dash dot",
        "loosely dash dot dot", "densely dash dot dot",
        "matrix of math nodes", "matrix of nodes",
        "middle color", "midway",
        "minimum height", "minimum size", "minimum width",
        "name", "name prefix", "name suffix", "near end", "near start",
        "node contents", "node distance", "nodes",
        "on background layer", "on grid", "opacity",
        "outer color", "outer sep", "outer xsep", "outer ysep",
        "overlay",
        "pattern", "pattern color",
        "pin", "pin distance", "pin edge", "pin position",
        "pos", "postaction", "preaction",
        "radius", "rectangle",
        "regular polygon", "regular polygon sides",
        "remember picture", "right color", "right delimiter", "right=of",
        "above=of", "below=of",
        "rotate", "rotate around", "rounded corners",
        "row sep",
        "samples", "scale", "scale around",
        "semithick",
        "shape", "shorten <", "shorten >",
        "sibling distance",
        "sloped", "smooth", "solid",
        "star", "star point height", "star point ratio",
        "start", "start angle", "end angle", "delta angle",
        "stealth", "step",
        "tension", "text", "text centered",
        "text depth", "text height", "text opacity",
        "text width",
        "thick", "thin", "tight", "to path", "bottom color", "top color",
        "transform canvas", "transform shape",
        "ultra thick", "ultra thin",
        "very thick", "very thin", "visible on",
        "x", "x radius", "xlabel", "xmax", "xmin", "xmode",
        "xscale", "xshift", "xslant", "xstep", "xtick", "xticklabels",
        "y", "y radius", "ylabel", "ymax", "ymin",
        "yscale", "yshift", "yslant", "ystep", "ytick", "yticklabels",
        "z", "z radius", "zlabel",

        // ── pgfplots axis ──
        "ymode", "zmode",
        "minor xtick", "minor ytick",
        "x tick label style", "y tick label style",
        "only marks", "mark", "mark size", "mark options",
        "no markers",
        "sharp plot",
        "const plot",
        "error bars",
        "title", "legend entries", "legend pos", "legend style",
        "colormap",
        "view",
        "axis equal", "axis equal image",
        "grid", "grid style",
        "major grid style", "minor grid style",
        "minor tick num",
        "enlargelimits",
        "width", "height",
        "axis x line", "axis y line", "axis z line",
        "axis line style",
        "zmin", "zmax", "ztick", "zticklabels", "zstep",

        // ── CircuitikZ keys (verified from pgfcircbipoles.tex) ──
        "american", "european",
        "wiper pos",

        // ── tikz-3dplot style keys (from styles section) ──
        "tdplot_main_coords",
        "tdplot_rotated_coords",
        "tdplot_screen_coords",
        "canvas is xy plane at z",
        "canvas is xz plane at y",
        "canvas is yz plane at x",
        "canvas is plane",
        "plane origin",
        "theta", "phi",

        // ── More general options ──
        "rounded rectangle",
        "trapezium", "trapezium angle",
        "cylinder",
        "cloud",
        "aspect",
        "polygon",
        "shape border rotate",
        "background rectangle",
        "show background rectangle",

        // ── Patterns ──
        "horizontal lines", "vertical lines",
        "north east lines", "north west lines",
        "crosshatch", "crosshatch dots",
        "bricks", "checkerboard",
        "grid", "dots",

        // ── Decorations ──
        "snake", "coil",
        "saw", "zigzag", "bumps",
        "brace",
        "markings",

        // ── More graph/tree ──
        "edge label",
        "edge from parent", "edge from parent path",
        "grow=up", "grow=down", "grow=left", "grow=right",
        "branch",
        "mindmap",
        "concept", "concept color",
        "counterclockwise", "clockwise",

        // ── More misc ──
        "alias",
        "behind path", "in front of path",
        "auto", "swap",
        "accepting", "initial",
        "loop above", "loop below", "loop left", "loop right",
        "every state",

        // ── Arrow specifications (verified PGF/TikZ keys) ──
        "->", "<-", "<->",

        // ── Predefined styles ──
        "help lines",
        "every picture", "every label", "every pin",
        "every to", "every scope",
        "label position",

        // ── More node shapes ──
        "coordinate", "rectangle split", "rectangle split parts",
        "circle split",
        "isosceles triangle", "semicircle", "circular sector",
        "single arrow", "double arrow",

        // ── More alignment / text ──
        "text ragged", "text badly ragged", "text badly centered",

        // ── More joins / caps ──
        "miter limit",

        // ── More misc ──
        "no draw", "no fill", "reset cm",
        "execute at begin node", "execute at end node",
        "circular drop shadow",
        "clip",

        // ── Path construction keywords ──
        "controls", "parabola",

        // ── Named colors (valid as direct bracket keys, e.g. \\draw[red]) ──
        "red", "green", "blue", "cyan", "magenta", "yellow",
        "black", "white", "gray", "darkgray", "lightgray",
        "brown", "lime", "olive", "orange", "pink", "purple",
        "teal", "violet",
    };
    return list;
}

inline const QStringList tikzAnchors() {
    static const QStringList list = {
        // ── Standard TikZ anchors ──
        "north", "south", "east", "west",
        "north east", "north west", "south east", "south west",
        "center", "base", "base east", "base west",
        "mid", "mid east", "mid west",
        "text", "text split",
        "left", "right", "top", "bottom",
        "apex", "corner 1", "corner 2", "tail",
        "input", "output", "input 1", "input 2", "output 1", "output 2",
        "angle",
        "150", "120", "90", "60", "30", "0",
        "210", "240", "270", "300", "330",

        // ── CircuitikZ anchors (verified from pgfcirc*.tex) ──
        "wiper", "W",
        "cathode", "anode",
        "gate", "G",
        "in", "in 1", "in 2", "out", "out 1", "out 2",
        "tap", "tap down", "tap up",
        "v+", "v-",
        "tip",
        "left", "right",
        "center", "north", "south", "east", "west",
    };
    return list;
}

inline const QStringList tikzKeyHandlers() {
    static const QStringList list = {
        "style",
        "default",
        "code",
        "append style",
        "prefix style",
        "initial",
        "add",
        "store in",
        "estore in",
        "value required",
        "value forbidden",
        "try",
        "retry",
        "search also",
        "cd",
        "handler",
    };
    return list;
}

inline const QStringList tikzColors() {
    static const QStringList list = {
        "red", "green", "blue", "cyan", "magenta", "yellow", "black",
        "white", "gray", "darkgray", "lightgray", "brown",
        "orange", "purple", "violet", "pink", "olive", "teal", "lime",
        "darkblue", "darkgreen", "darkred", "darkcyan", "darkmagenta",
        "darkyellow", "darkorange", "darkpurple", "darkviolet",
        "lightblue", "lightgreen", "lightred", "lightcyan", "lightmagenta",
        "lightyellow", "lightorange", "lightpink",
        "copper", "gold", "silver", "bronze",
    };
    return list;
}

inline const QStringList tikzArrows() {
    static const QStringList list = {
        "stealth", "stealth'", "latex", "latex reversed", "to", "to reversed",
        "angle 90", "angle 60", "angle 45",
        "triangle 90", "triangle 60", "triangle 45",
        "open triangle 90", "open triangle 60", "open triangle 45",
        "Circle", "Diamond", "Square",
        "Bar", "Bracket", "Parenthesis",
        "Round Cap", "Butt Cap", "Triangle Cap",
        "Fast Round", "Fast Triangle",
        "Hooks", "Implies", "Straight Barb",
        "Arc Barb", "Classical TikZ Rightarrow",
        "Computer Modern Rightarrow",
        "Stealth", "Latex", "To",
    };
    return list;
}

inline const QStringList tikzLineTypes() {
    static const QStringList list = {
        "solid", "dashed", "dotted", "dash dot", "dash dot dot",
        "loosely dashed", "densely dashed",
        "loosely dotted", "densely dotted",
        "loosely dash dot", "densely dash dot",
        "loosely dash dot dot", "densely dash dot dot",
    };
    return list;
}

inline const QStringList tikzLineWidths() {
    static const QStringList list = {
        "ultra thin", "very thin", "thin", "semithick",
        "thick", "very thick", "ultra thick",
    };
    return list;
}

inline const QStringList tikzMathFunctions() {
    static const QStringList list = {
        "abs", "acos", "add", "angle", "asin", "atan", "atan2",
        "ceil", "cit", "clip", "cm", "cos", "cosec", "cosh",
        "cot", "deg", "depth", "dim", "divide", "dotproduct",
        "e", "exp",
        "factorial", "floor", "format",
        "frac", "gcd", "height",
        "in", "int", "intersection", "isodd",
        "kil", "length", "ln", "log10", "log2",
        "max", "min", "mod", "multiply",
        "not", "note",
        "pi", "pow", "power",
        "pt",
        "rad", "rand", "random", "rdiv", "real", "record", "res",
        "round", "scalar", "sec", "sign", "sin", "sinh",
        "sqrt", "subtract", "sum",
        "tan", "tanh", "true", "turn",
        "value", "vecilen", "width",
    };
    return list;
}

inline QStringList tikzLineWidthValues() {
    return {"0.2pt","0.4pt","0.5pt","0.6pt","0.8pt","1pt","1.2pt","1.5pt","2pt","2.5pt","3pt","4pt","5pt","6pt","8pt","10pt"};
}

inline QStringList tikzArrowValues() {
    return {"stealth", "latex", "to", "angle 90", "angle 60", "triangle 90",
            "Triangle", "Circle", "Diamond", "Square", "Bar", "Bracket",
            "Parenthesis", "Round Cap", "Butt Cap", "Triangle Cap",
            "Fast Round", "Fast Triangle", "Straight Barb", "Arc Barb",
            "Computer Modern Rightarrow", "Implies", "Hooks", "Classical TikZ Rightarrow"};
}

using WordPair = QPair<QString, QStringList>;

inline QVector<WordPair> tikzValueHints() {
    static const QVector<WordPair> hints = {
        // ── Colors ──
        {"color", tikzColors()},
        {"draw", tikzColors()},
        {"fill", tikzColors()},
        {"left color", tikzColors()},
        {"right color", tikzColors()},
        {"middle color", tikzColors()},
        {"top color", tikzColors()},
        {"bottom color", tikzColors()},
        {"inner color", tikzColors()},
        {"outer color", tikzColors()},
        {"ball color", tikzColors()},
        {"pattern color", tikzColors()},
        {"text", tikzColors()},
        {"concept color", tikzColors()},

        // ── Arrows ──
        {"arrow", tikzArrowValues()},
        {"-", tikzArrowValues()},
        {">", tikzArrowValues()},
        {">=", tikzArrowValues()},
        {"mark", {"*", "x", "+", "-", "|", "o", "asterisk", "star",
                  "oplus", "otimes", "square", "square*",
                  "triangle", "triangle*", "diamond", "diamond*",
                  "pentagon", "pentagon*", "text", "none"}},

        // ── Patterns ──
        {"pattern", {"horizontal lines", "vertical lines",
                     "north east lines", "north west lines",
                     "crosshatch", "crosshatch dots",
                     "bricks", "checkerboard",
                     "grid", "dots", "fivepointed stars",
                     "sixpointed stars"}},

        // ── Line widths ──
        {"line width", tikzLineWidthValues()},
        {"ultra thin", {}},
        {"very thin", {}},
        {"thin", {}},
        {"semithick", {}},
        {"thick", {}},
        {"very thick", {}},
        {"ultra thick", {}},

        // ── Line caps/joins ──
        {"cap", {"butt", "round", "rect"}},
        {"line cap", {"butt", "round", "rect"}},
        {"join", {"round", "bevel", "miter"}},
        {"line join", {"round", "bevel", "miter"}},

        // ── Alignment ──
        {"align", {"left", "center", "right", "justify", "none"}},
        {"anchor", tikzAnchors()},
        {"pin position", tikzAnchors()},

        // ── Bend ──
        {"bend left", {"90","60","45","30","15"}},
        {"bend right", {"90","60","45","30","15"}},
        {"bend angle", {"90","60","45","30","15"}},

        // ── Arc ──
        {"start angle", {"0","30","45","60","90","120","135","150","180","210","225","240","270","300","315","330"}},
        {"end angle", {"0","30","45","60","90","120","135","150","180","210","225","240","270","300","315","330","360"}},

        // ── Curve to-path ──
        {"in", {"0","30","45","60","90","120","135","150","180","-135","-120","-90","-60","-45","-30"}},
        {"out", {"0","30","45","60","90","120","135","150","180","-135","-120","-90","-60","-45","-30"}},

        // ── Rotation ──
        {"rotate", {"90","180","270","45","60","30","15"}},

        // ── Scale ──
        {"scale", {"0.5","0.8","1.0","1.2","1.5","2.0","3.0"}},

        // ── Opacity ──
        {"opacity", {"0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"}},
        {"draw opacity", {"0.3","0.5","0.7","1.0"}},
        {"fill opacity", {"0.3","0.5","0.7","1.0"}},
        {"text opacity", {"0.5","0.7","1.0"}},

        // ── Size / length ──
        {"text width", {"2cm","3cm","4cm","5cm","6cm","8cm","10cm"}},
        {"minimum width", {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"}},
        {"minimum height", {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"}},
        {"minimum size", {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"}},
        {"inner sep", {"0pt","1pt","2pt","3pt","4pt","5pt","6pt","8pt","10pt"}},
        {"outer sep", {"0pt","1pt","2pt","3pt","4pt","5pt"}},
        {"inner xsep", {"0pt","2pt","4pt","6pt","8pt","10pt"}},
        {"inner ysep", {"0pt","2pt","4pt","6pt","8pt","10pt"}},
        {"node distance", {"0.5cm","1cm","1.5cm","2cm","2.5cm","3cm","4cm"}},
        {"level distance", {"0.5cm","1cm","1.5cm","2cm","3cm"}},
        {"sibling distance", {"0.5cm","1cm","1.5cm","2cm","3cm"}},
        {"rounded corners", {"1pt","2pt","3pt","4pt","5pt","8pt","10pt"}},
        {"double distance", {"1pt","1.5pt","2pt","3pt","4pt","5pt"}},
        {"shorten <", {"1pt","2pt","3pt","5pt","8pt","10pt"}},
        {"shorten >", {"1pt","2pt","3pt","5pt","8pt","10pt"}},
        {"radius", {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"}},

        // ── pgfplots ──
        {"samples", {"50","100","200","500","1000"}},
        {"domain", {"0:10","0:1","-5:5","-10:10","-pi:pi","0:2*pi"}},
        {"regular polygon sides", {"3","4","5","6","7","8","10","12"}},
        {"star point ratio", {"0.3","0.4","0.5","0.6","0.8"}},

        // ── More pgfplots keys ──
        {"mark size", {"1pt","2pt","3pt","4pt","5pt","6pt","8pt"}},
        {"xmode", {"normal", "log"}},
        {"ymode", {"normal", "log"}},
        {"legend pos", {"north east", "north west", "south east", "south west",
                        "outer north east"}},
    };
    return hints;
}

inline QStringList allCompletableWords() {
    QStringList result;
    QSet<QString> seen;
    auto appendUnique = [&](const QStringList &words) {
        for (const QString &w : words) {
            QString lower = w.toLower();
            if (!seen.contains(lower)) {
                seen.insert(lower);
                result << w;
            }
        }
    };
    appendUnique(tikzCommands());
    appendUnique(tikzOptions());
    appendUnique(tikzAnchors());
    appendUnique(tikzColors());
    appendUnique(tikzArrows());
    appendUnique(tikzLineTypes());
    appendUnique(tikzLineWidths());
    appendUnique(tikzMathFunctions());
    appendUnique(tikzArrowValues());
    appendUnique(tikzLineWidthValues());
    result.sort(Qt::CaseInsensitive);
    return result;
}

inline const QStringList &pgfKeysPathCommands() {
    static const QStringList list = {
        "/tikz", "/pgf", "/pgfplots", "/graph",
        "/tikz/every picture", "/tikz/every node", "/tikz/every path",
        "/tikz/every edge", "/tikz/every label", "/tikz/every pin",
        "/tikz/every graph", "/tikz/every plot",
        "/pgf/number format",
        "/pgfplots/legend",
        "/circuitikz",
    };
    return list;
}

} // namespace TikzWords
