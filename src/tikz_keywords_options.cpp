#include "tikz_keywords_data.h"

namespace TikzKeywords {

void registerGeneralOptions(Vec &db)
{
    // ═══════════════════════════════════════════════════
    // ── GENERAL OPTIONS (universal tikz keys) ──
    // ═══════════════════════════════════════════════════

    // Positioning
    addBuiltin(db, "above",        C::Option, {}, {"node","pic"});
    addBuiltin(db, "above left",   C::Option, {}, {"node","pic"});
    addBuiltin(db, "above right",  C::Option, {}, {"node","pic"});
    addBuiltin(db, "below",        C::Option, {}, {"node","pic"});
    addBuiltin(db, "below left",   C::Option, {}, {"node","pic"});
    addBuiltin(db, "below right",  C::Option, {}, {"node","pic"});
    addBuiltin(db, "left",         C::Option, {}, {"node","pic"});
    addBuiltin(db, "right",        C::Option, {}, {"node","pic"});
    addBuiltin(db, "left=of",      C::Option, {"tikzpicture","scope"}, {"node","pic"}, {}, {"positioning"});
    addBuiltin(db, "right=of",     C::Option, {"tikzpicture","scope"}, {"node","pic"}, {}, {"positioning"});
    addBuiltin(db, "above=of",     C::Option, {"tikzpicture","scope"}, {"node","pic"}, {}, {"positioning"});
    addBuiltin(db, "below=of",     C::Option, {"tikzpicture","scope"}, {"node","pic"}, {}, {"positioning"});
    addBuiltin(db, "on grid",      C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"positioning"});
    addBuiltin(db, "node distance",C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","2.5cm","3cm","4cm"}, {"positioning"});

    // Common transforms
    addBuiltin(db, "rotate",          C::Option, {}, {},
               {"90","180","270","45","60","30","15"});
    addBuiltin(db, "rotate around",   C::Option, {});
    addBuiltin(db, "scale",           C::Option, {}, {},
               {"0.5","0.8","1.0","1.2","1.5","2.0","3.0"});
    addBuiltin(db, "scale around",    C::Option, {});
    addBuiltin(db, "xscale",          C::Option, {});
    addBuiltin(db, "yscale",          C::Option, {});
    addBuiltin(db, "xshift",          C::Option, {});
    addBuiltin(db, "yshift",          C::Option, {});
    addBuiltin(db, "xslant",          C::Option, {});
    addBuiltin(db, "yslant",          C::Option, {});
    addBuiltin(db, "transform canvas",C::Option, {});
    addBuiltin(db, "transform shape", C::Option, {});
    addBuiltin(db, "reset cm",        C::Option, {});
    addBuiltin(db, "shift",           C::Option, {});

    // Color / opacity
    addBuiltin(db, "color",           C::Option, {}, {"draw","path","fill","node"});
    addBuiltin(db, "draw",            C::Option, {}, {"draw","path","node","edge"});
    addBuiltin(db, "fill",            C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "draw opacity",    C::Option, {}, {"draw","path"},
               {"0.3","0.5","0.7","1.0"});
    addBuiltin(db, "fill opacity",    C::Option, {}, {"draw","path","node"},
               {"0.3","0.5","0.7","1.0"});
    addBuiltin(db, "text opacity",    C::Option, {}, {"node"},
               {"0.5","0.7","1.0"});
    addBuiltin(db, "opacity",         C::Option, {}, {"draw","path","node"},
               {"0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"});
    addBuiltin(db, "left color",      C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "right color",     C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "middle color",    C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "top color",       C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "bottom color",    C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "inner color",     C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "outer color",     C::Option, {}, {"draw","path","shade","shadedraw"});
    addBuiltin(db, "ball color",      C::Option, {}, {"node"});
    addBuiltin(db, "pattern color",   C::Option, {}, {"draw","path","fill","filldraw"});

    // Line / stroke
    addBuiltin(db, "line width",          C::Option, {}, {"draw","path","filldraw"},
               {"0.2pt","0.4pt","0.5pt","0.6pt","0.8pt","1pt","1.2pt","1.5pt","2pt","2.5pt","3pt","4pt","5pt","6pt","8pt","10pt"});
    addBuiltin(db, "line cap",            C::Option, {}, {"draw","path"},
               {"butt","round","rect"});
    addBuiltin(db, "line join",           C::Option, {}, {"draw","path"},
               {"round","bevel","miter"});
    addBuiltin(db, "miter limit",         C::Option, {}, {"draw","path"});
    addBuiltin(db, "double",              C::Option, {}, {"draw","path"});
    addBuiltin(db, "double distance",     C::Option, {}, {"draw","path"},
               {"1pt","1.5pt","2pt","3pt","4pt","5pt"});
    addBuiltin(db, "double equal sign distance", C::Option, {}, {"draw","path"});
    addBuiltin(db, "dash",                C::Option, {}, {"draw","path"});
    addBuiltin(db, "dash phase",          C::Option, {}, {"draw","path"});
    addBuiltin(db, "solid",               C::Option, {}, {"draw","path"});
    addBuiltin(db, "rounded corners",     C::Option, {}, {"draw","path","node"},
               {"1pt","2pt","3pt","4pt","5pt","8pt","10pt"});
    addBuiltin(db, "cap",                 C::Option, {}, {"draw","path"}, {"butt","round","rect"}, {});
    addBuiltin(db, "join",                C::Option, {}, {"draw","path"}, {"round","bevel","miter"}, {});
    addBuiltin(db, "shorten <",           C::Option, {}, {"draw","path"},
               {"1pt","2pt","3pt","5pt","8pt","10pt"});
    addBuiltin(db, "shorten >",           C::Option, {}, {"draw","path"},
               {"1pt","2pt","3pt","5pt","8pt","10pt"});

    // Arrows key
    addBuiltin(db, "arrow",            C::Option, {}, {"draw","path"},
               {"stealth","latex","to","angle 90","angle 60","triangle 90",
                "Triangle","Circle","Diamond","Square","Bar","Bracket",
                "Parenthesis","Round Cap","Butt Cap","Triangle Cap",
                "Fast Round","Fast Triangle","Straight Barb","Arc Barb",
                "Computer Modern Rightarrow","Implies","Hooks","Classical TikZ Rightarrow"});

    // Node-specific
    addBuiltin(db, "shape",            C::Option, {"tikzpicture","scope"}, {"node"},
               {"rectangle","circle","ellipse","diamond","regular polygon",
                "isosceles triangle","trapezium","semicircle","circular sector",
                "cylinder","single arrow","double arrow","cloud","star",
                "rectangle split","circle split","rounded rectangle"});
    addBuiltin(db, "anchor",           C::Option, {}, {"node"},
               {"north","south","east","west","north east","north west",
                "south east","south west","center","base","base east","base west",
                "mid","mid east","mid west","text"});
    addBuiltin(db, "label",            C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "pin",              C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "pin distance",     C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "pin edge",         C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "pin position",     C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "name",             C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "name prefix",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "name suffix",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "alias",            C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "execute at begin node", C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "execute at end node",   C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "late options",     C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "behind path",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "in front of path", C::Option, {"tikzpicture","scope"}, {"node"});

    // Node dimensions
    addBuiltin(db, "minimum width",    C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"});
    addBuiltin(db, "minimum height",   C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"});
    addBuiltin(db, "minimum size",     C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"});
    addBuiltin(db, "inner sep",        C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","1pt","2pt","3pt","4pt","5pt","6pt","8pt","10pt"});
    addBuiltin(db, "inner xsep",       C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","2pt","4pt","6pt","8pt","10pt"});
    addBuiltin(db, "inner ysep",       C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","2pt","4pt","6pt","8pt","10pt"});
    addBuiltin(db, "outer sep",        C::Option, {"tikzpicture","scope"}, {"node"},
               {"0pt","1pt","2pt","3pt","4pt","5pt"});
    addBuiltin(db, "outer xsep",       C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "outer ysep",       C::Option, {"tikzpicture","scope"}, {"node"});

    // Text in nodes
    addBuiltin(db, "text",             C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text width",       C::Option, {"tikzpicture","scope"}, {"node"},
               {"2cm","3cm","4cm","5cm","6cm","8cm","10cm"});
    addBuiltin(db, "text height",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text depth",       C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text centered",    C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text ragged",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text badly ragged",      C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "text badly centered",    C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "align",            C::Option, {"tikzpicture","scope"}, {"node"},
               {"left","center","right","justify","none"});
    addBuiltin(db, "font",             C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "fontsize",         C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "node contents",    C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "node distance",    C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","2.5cm","3cm","4cm"});

    // Path decorations
    addBuiltin(db, "decorate",        C::Option, {}, {"draw","path"}, {}, {"decorations.pathmorphing","decorations.pathreplacing","decorations.markings"});
    addBuiltin(db, "decoration",      C::Option, {}, {"draw","path"},
               {"snake","coil","saw","zigzag","bumps","brace","markings"},
               {"decorations.pathmorphing","decorations.pathreplacing","decorations.markings"});

    // Bends / curves
    addBuiltin(db, "bend left",        C::Option, {}, {"draw","path","edge"},
               {"90","60","45","30","15"});
    addBuiltin(db, "bend right",       C::Option, {}, {"draw","path","edge"},
               {"90","60","45","30","15"});
    addBuiltin(db, "bend angle",       C::Option, {}, {"draw","path","edge"},
               {"90","60","45","30","15"});
    addBuiltin(db, "bend at start",    C::Option, {}, {"draw","path","edge"});
    addBuiltin(db, "bend at end",      C::Option, {}, {"draw","path","edge"});
    addBuiltin(db, "bend pos",         C::Option, {}, {"draw","path","edge"});
    addBuiltin(db, "in",               C::Option, {}, {"draw","path"},
               {"0","30","45","60","90","120","135","150","180","-135","-120","-90","-60","-45","-30"});
    addBuiltin(db, "out",              C::Option, {}, {"draw","path"},
               {"0","30","45","60","90","120","135","150","180","-135","-120","-90","-60","-45","-30"});
    addBuiltin(db, "in looseness",     C::Option, {}, {"draw","path"});
    addBuiltin(db, "out looseness",    C::Option, {}, {"draw","path"});
    addBuiltin(db, "looseness",        C::Option, {}, {"draw","path"});
    addBuiltin(db, "auto",             C::Option, {}, {"draw","path","node","edge"});
    addBuiltin(db, "swap",             C::Option, {}, {"draw","path","node","edge"});
    addBuiltin(db, "sloped",           C::Option, {}, {"draw","path","node","edge"});

    // Path positioning
    addBuiltin(db, "pos",              C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "midway",           C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "near start",       C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "near end",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "very near start",  C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "very near end",    C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "at start",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "at end",           C::Option, {}, {"draw","path","node"});

    // Misc drawing
    addBuiltin(db, "smooth",           C::Option, {}, {"draw","path"});
    addBuiltin(db, "tension",          C::Option, {}, {"draw","path"});
    addBuiltin(db, "append after command", C::Option, {}, {"node","pic","path"});
    addBuiltin(db, "preaction",        C::Option, {}, {"draw","path"});
    addBuiltin(db, "postaction",       C::Option, {}, {"draw","path"});
    addBuiltin(db, "overlay",          C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "remember picture", C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "baseline",         C::Option, {"tikzpicture"});
    addBuiltin(db, "every picture",    C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every node",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every edge",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every loop",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every label",      C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every pin",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every to",         C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "every scope",      C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "label position",   C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "drop shadow",      C::Option, {}, {"node"}, {}, {"shadows"});
    addBuiltin(db, "circular drop shadow", C::Option, {}, {"node"}, {}, {"shadows"});
    addBuiltin(db, "visible on",       C::Option, {}, {"node","draw"}, {}, {"overlay-beamer-styles"});
    addBuiltin(db, "no draw",          C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "no fill",          C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "clip",             C::Option, {"tikzpicture","scope"}, {"draw","path"});

    // Path construction keywords (also available as options for some contexts)
    addBuiltin(db, "controls",         C::Option, {}, {"draw","path"});
    addBuiltin(db, "parabola",         C::Option, {}, {"draw","path"});

    // Fill rules
    addBuiltin(db, "nonzero rule",     C::Option, {}, {"draw","path","fill"});
    addBuiltin(db, "even odd rule",    C::Option, {}, {"draw","path","fill"});

    // Arc
    addBuiltin(db, "start angle",      C::Option, {}, {"draw","path"},
               {"0","30","45","60","90","120","135","150","180","210","225","240","270","300","315","330"});
    addBuiltin(db, "end angle",        C::Option, {}, {"draw","path"},
               {"0","30","45","60","90","120","135","150","180","210","225","240","270","300","315","330","360"});
    addBuiltin(db, "delta angle",      C::Option, {}, {"draw","path"});

    // Ellipse / arc dimensions
    addBuiltin(db, "x radius",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "y radius",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "z radius",         C::Option, {}, {"draw","path","node"});
    addBuiltin(db, "radius",           C::Option, {}, {"draw","path","node"},
               {"0.5cm","1cm","1.5cm","2cm","3cm","4cm","5cm"});

    // Matrix
    addBuiltin(db, "matrix of nodes",       C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "matrix of math nodes",  C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "column sep",            C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "row sep",               C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "nodes",                 C::Option, {}, {"draw","path"}, {}, {"matrix"});
    addBuiltin(db, "delimiters",            C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "left delimiter",        C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "right delimiter",       C::Option, {}, {"node"}, {}, {"matrix"});

    // Angles library
    addBuiltin(db, "angle radius",     C::Option, {}, {},
               {"0.3cm","0.5cm","0.8cm","1cm","1.5cm","2cm","3cm"}, {"angles"});
    addBuiltin(db, "angle eccentricity", C::Option, {}, {},
               {"0.3","0.5","0.6","0.8","1.0","1.2","1.5"}, {"angles"});
    addBuiltin(db, "right angle",      C::Option, {}, {}, {}, {"angles"});
    addBuiltin(db, "pic text",         C::Option, {}, {}, {}, {"angles","quotes"});

    // Intersections library
    addBuiltin(db, "name path",         C::Option, {}, {"draw","path"}, {}, {"intersections"});
    addBuiltin(db, "name path global",  C::Option, {}, {"draw","path"}, {}, {"intersections"});
    addBuiltin(db, "name intersections",C::Option, {}, {"draw","path"}, {}, {"intersections"});
    addBuiltin(db, "of",                C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "by",                C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "sort by",           C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "total",             C::Option, {}, {}, {}, {"intersections"});

    // Shape-specific options
    addBuiltin(db, "regular polygon sides", C::Option, {"tikzpicture","scope"}, {"node"},
               {"3","4","5","6","7","8","10","12"}, {"shapes.geometric"});
    addBuiltin(db, "star point ratio",      C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.3","0.4","0.5","0.6","0.8"}, {"shapes.geometric"});
    addBuiltin(db, "star point height",     C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "trapezium angle",       C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "aspect",                C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.geometric"});
    addBuiltin(db, "shape border rotate",   C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "rectangle split parts", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});

    // Tree / graph
    addBuiltin(db, "grow",              C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow cyclic",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow=up",           C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow=down",         C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow=left",         C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "grow=right",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "level distance",    C::Option, {"tikzpicture","scope"},
               {"0.5cm","1cm","1.5cm","2cm","3cm"});
    addBuiltin(db, "sibling distance",  C::Option, {"tikzpicture","scope"},
               {"0.5cm","1cm","1.5cm","2cm","3cm"});
    addBuiltin(db, "branch",            C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "edge from parent",  C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "edge from parent path", C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "edge label",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "counterclockwise",  C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "clockwise",         C::Option, {"tikzpicture","scope"}, {"node"});

    // Mindmap
    addBuiltin(db, "mindmap",           C::Option, {"tikzpicture"});
    addBuiltin(db, "concept",           C::Option, {"tikzpicture"}, {}, {}, {"mindmap"});
    addBuiltin(db, "concept color",     C::Option, {"tikzpicture"}, {}, {}, {"mindmap"});

    // Automata
    addBuiltin(db, "accepting",         C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "initial",           C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "every state",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "loop above",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "loop below",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "loop left",         C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "loop right",        C::Option, {"tikzpicture","scope"});

    // Background layer
    addBuiltin(db, "on background layer",  C::Option, {"tikzpicture","scope"}, {}, {}, {"backgrounds"});
    addBuiltin(db, "show background rectangle", C::Option, {"tikzpicture","scope"}, {}, {}, {"backgrounds"});
    addBuiltin(db, "background rectangle", C::Option, {"tikzpicture","scope"}, {}, {}, {"backgrounds"});

    // Patterns as option
    addBuiltin(db, "pattern",           C::Option, {}, {"draw","path","fill","filldraw"},
               {"horizontal lines","vertical lines","north east lines","north west lines",
                "crosshatch","crosshatch dots","bricks","checkerboard",
                "grid","dots","fivepointed stars","sixpointed stars"});

    // Predefined styles
    addBuiltin(db, "help lines",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "information text",  C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "to path",           C::Option, {}, {"draw","path"});

    // Coordinate systems
    addBuiltin(db, "x",                 C::Option, {}, {});
    addBuiltin(db, "y",                 C::Option, {}, {});
    addBuiltin(db, "z",                 C::Option, {}, {});
}

} // namespace TikzKeywords
