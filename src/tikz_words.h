#pragma once
#include <QStringList>
#include <QVector>
#include <QPair>

namespace TikzWords {

inline const QStringList tikzCommands() {
    static const QStringList list = {
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
        "xdef"
    };
    return list;
}

inline const QStringList tikzEnvironments() {
    static const QStringList list = {
        "tikzpicture", "scope", "pgfonlayer",
        "axis", "semilogxaxis", "semilogyaxis", "loglogaxis",
        "polaraxis", "smithchart", "ternaryaxis",
        "groupplot", "polarplot",
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
        "align", "equation", "displaymath", "math",
        "abstract", "proof", "theorem", "lemma"
    };
    return list;
}

inline const QStringList tikzOptions() {
    static const QStringList list = {
        "above", "above left", "above right",
        "align", "allow upside down", "anchor", "angle", "append after command",
        "arrow",
        "ball color", "baseline", "below", "below left", "below right",
        "bend angle", "bend at end", "bend at start", "bend left", "bend pos",
        "bend right", "blend mode",
        "cap", "cells", "check",
        "circle", "circular drop shadow",
        "color", "column sep", "columns",
        "dash", "dash dot", "dash dot dot", "dash phase", "dashed", "densely dash dot",
        "densely dash dot dot", "densely dashed", "densely dotted",
        "decorate", "decoration",
        "diamond", "distance", "domain", "dot", "dotted", "double", "double distance",
        "double equal sign distance",
        "draw", "draw opacity", "drop shadow",
        "ellipse", "end angle", "end point",
        "every edge", "every loop", "every node",
        "fill", "fill opacity", "filldraw", "font", "fontsize",
        "green", "grow", "grow cyclic",
        "huge", "hyper",
        "in", "inner color", "inner sep", "inner xsep", "inner ysep",
        "join", "join round",
        "label",
        "late options", "left", "left color", "left delimiter", "left=of",
        "level distance", "line cap", "line join", "line width", "looseness", "loosely dash dot",
        "loosely dash dot dot", "loosely dashed", "loosely dotted",
        "matrix of math nodes", "matrix of nodes",
        "mid", "middle color", "midway", "minimum height", "minimum size", "minimum width",
        "name", "name prefix", "name suffix", "near end", "near start",
        "node contents", "node distance", "nodes",
        "on background layer", "on grid", "opacity", "outer color", "outer sep",
        "outer xsep", "outer ysep", "outline", "overlay",
        "pattern", "pattern color", "pattern type",
        "pin", "pin distance", "pin edge", "pin position",
        "pos", "postaction", "preaction", "prefix after command",
        "radius", "rectangle", "red",
        "regular polygon", "regular polygon sides",
        "remember picture", "right", "right color", "right delimiter", "right=of",
        "rotate", "rotate around", "rounded corners", "row sep", "rows",
        "samples", "scale", "scale around", "semithick",
        "shape", "shorten <", "shorten >", "sibling distance",
        "sin", "size", "sloped", "smooth", "solid",
        "star", "star point height", "star point ratio", "start", "start angle", "start point",
        "stealth", "step",
        "tension", "text", "text centered", "text depth", "text height", "text opacity",
        "text width", "thick", "thin", "tight", "to path", "top color",
        "transform", "transform canvas", "transform shape",
        "triangle",
        "ultra thick", "ultra thin",
        "variable",
        "very thick", "very thin", "visible on",
        "x", "x radius", "xcomb", "xlabel", "xmax", "xmin", "xmode",
        "xscale", "xshift", "xslant", "xstep", "xtick", "xticklabels",
        "y", "y radius", "ycomb", "ylabel", "ymax", "ymin",
        "yscale", "yshift", "yslant", "ystep", "ytick", "yticklabels",
        "z", "zlabel"
    };
    return list;
}

inline const QStringList tikzAnchors() {
    static const QStringList list = {
        "north", "south", "east", "west",
        "north east", "north west", "south east", "south west",
        "center", "base", "base east", "base west",
        "mid", "mid east", "mid west",
        "text", "text split",
        "left", "right", "top", "bottom",
        "apex", "corner 1", "corner 2", "tail",
        "input", "output", "input 1", "input 2", "output 1", "output 2",
        "north east corner", "north west corner", "south east corner", "south west corner",
        "angle"
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
        "copper", "gold", "silver", "bronze"
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
        "Arc Barb", "Classical TikZ Rightarrow"
    };
    return list;
}

inline const QStringList tikzLineTypes() {
    static const QStringList list = {
        "solid", "dashed", "dotted", "dash dot", "dash dot dot",
        "loosely dashed", "densely dashed",
        "loosely dotted", "densely dotted",
        "loosely dash dot", "densely dash dot",
        "loosely dash dot dot", "densely dash dot dot"
    };
    return list;
}

inline const QStringList tikzLineWidths() {
    static const QStringList list = {
        "ultra thin", "very thin", "thin", "semithick",
        "thick", "very thick", "ultra thick"
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
        "value", "vecilen", "width"
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
        {"arrow", tikzArrowValues()},
        {"line width", tikzLineWidthValues()},
        {"ultra thin", {}},
        {"very thin", {}},
        {"thin", {}},
        {"semithick", {}},
        {"thick", {}},
        {"very thick", {}},
        {"ultra thick", {}},
        {"cap", {"butt", "round", "rect"}},
        {"join", {"round", "bevel", "miter"}},
        {"align", {"left", "center", "right", "justify", "none"}},
        {"anchor", tikzAnchors()},
        {"pin position", tikzAnchors()},
        {"bend left", {"90","60","45","30","15"}},
        {"bend right", {"90","60","45","30","15"}},
        {"rotate", {"90","180","270","45","60","30","15"}},
        {"scale", {"0.5","0.8","1.0","1.2","1.5","2.0","3.0"}},
        {"opacity", {"0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"}},
        {"text width", {"2cm","3cm","4cm","5cm","6cm","8cm","10cm"}},
        {"minimum width", {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"}},
        {"minimum height", {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"}},
        {"inner sep", {"0pt","1pt","2pt","3pt","4pt","5pt","6pt","8pt","10pt"}},
        {"outer sep", {"0pt","1pt","2pt","3pt","4pt","5pt"}},
        {"node distance", {"0.5cm","1cm","1.5cm","2cm","2.5cm","3cm","4cm"}},
        {"level distance", {"0.5cm","1cm","1.5cm","2cm","3cm"}},
        {"sibling distance", {"0.5cm","1cm","1.5cm","2cm","3cm"}},
        {"rounded corners", {"1pt","2pt","3pt","4pt","5pt","8pt","10pt"}},
        {"double distance", {"1pt","1.5pt","2pt","3pt","4pt","5pt"}},
        {"shorten <", {"1pt","2pt","3pt","5pt","8pt","10pt"}},
        {"shorten >", {"1pt","2pt","3pt","5pt","8pt","10pt"}},
        {"samples", {"50","100","200","500","1000"}},
        {"domain", {"0:10","0:1","-5:5","-10:10","-pi:pi"}},
        {"regular polygon sides", {"3","4","5","6","7","8","10","12"}},
        {"star point ratio", {"0.3","0.4","0.5","0.6","0.8"}},
        {"draw opacity", {"0.3","0.5","0.7","1.0"}},
        {"fill opacity", {"0.3","0.5","0.7","1.0"}},
        {"text opacity", {"0.5","0.7","1.0"}},
    };
    return hints;
}

inline QStringList allCompletableWords() {
    QStringList result;
    result << tikzCommands() << tikzOptions() << tikzAnchors()
           << tikzColors() << tikzArrows()
           << tikzLineTypes() << tikzLineWidths()
           << tikzMathFunctions() << tikzArrowValues()
           << tikzLineWidthValues();
    result.removeDuplicates();
    result.sort(Qt::CaseInsensitive);
    return result;
}

inline const QStringList &pgfKeysPathCommands() {
    static const QStringList list = {
        "/tikz", "/pgf", "/pgfplots", "/graph",
        "/tikz/every picture", "/tikz/every node", "/tikz/every path",
        "/tikz/every edge", "/tikz/every label", "/tikz/every pin",
        "/tikz/every graph", "/tikz/every plot"
    };
    return list;
}

} // namespace TikzWords
