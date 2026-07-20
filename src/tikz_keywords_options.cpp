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
    addBuiltin(db, "label",            C::Option, {"tikzpicture","scope"}, {"node"},
               {"above","below","left","right",
                "above left","above right","below left","below right",
                "center","north","south","east","west",
                "north east","north west","south east","south west"});
    addBuiltin(db, "pin",              C::Option, {"tikzpicture","scope"}, {"node"},
               {"above","below","left","right",
                "above left","above right","below left","below right"});
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
    addBuiltin(db, "font",             C::Option, {"tikzpicture","scope"}, {"node"},
               {"\\rmfamily","\\sffamily","\\ttfamily","\\bfseries","\\mdseries",
                "\\upshape","\\itshape","\\slshape","\\scshape",
                "\\tiny","\\scriptsize","\\footnotesize","\\small",
                "\\normalsize","\\large","\\Large","\\LARGE","\\huge","\\Huge"});
    addBuiltin(db, "node font",        C::Option, {"tikzpicture","scope"}, {"node"},
               {"\\rmfamily","\\sffamily","\\ttfamily","\\bfseries","\\mdseries",
                "\\upshape","\\itshape","\\slshape","\\scshape",
                "\\tiny","\\scriptsize","\\footnotesize","\\small",
                "\\normalsize","\\large","\\Large","\\LARGE","\\huge","\\Huge"});
    addBuiltin(db, "node contents",    C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "node distance",    C::Option, {"tikzpicture","scope"}, {"node"},
               {"0.5cm","1cm","1.5cm","2cm","2.5cm","3cm","4cm"});

    // Path decorations
    addBuiltin(db, "decorate",        C::Option, {}, {"draw","path"}, {}, {"decorations.pathmorphing","decorations.pathreplacing","decorations.markings"});
    // Hints intentionally empty — eqCandidatesForKey falls through to the
    // "decoration" special case which returns ALL decoration names via
    // tikzDecorationNames() (26 entries), not a hardcoded subset.
    addBuiltin(db, "decoration",      C::Option, {}, {"draw","path"}, {},
               {"decorations.pathmorphing","decorations.pathreplacing","decorations.markings"});

    // ── Decoration sub-option keys (common to all decorations) ──
    // Verified against pgfmoduledecorations.code.tex (core) and
    // tikzlibrarydecorations.code.tex (TikZ frontend). These are
    // always available once any decoration is used.
    {
        const char *common[] = {
            "amplitude","segment length","angle","aspect",
            "start radius","end radius","radius",
            "path has corners","reverse path",
            "raise","mirror","transform",
            "pre","post","pre length","post length",
            nullptr};
        for (int i = 0; common[i]; i++)
            addBuiltin(db, common[i], C::Option, {}, {"draw","path"});
    }

    // ── decorations.shapes sub-keys ──
    // Verified against pgflibrarydecorations.shapes.code.tex.
    {
        const char *keys[] = {
            "shape","shape start width","shape start height",
            "shape end width","shape end height","anchor",
            nullptr};
        for (int i = 0; keys[i]; i++)
            addBuiltin(db, keys[i], C::Option, {}, {"draw","path"}, {},
                       {"decorations.shapes"});
    }

    // ── decorations.text sub-keys ──
    // Verified against pgflibrarydecorations.text.code.tex.
    {
        addBuiltin(db, "text", C::Option, {}, {"draw","path"}, {},
                   {"decorations.text"});
        addBuiltin(db, "text align", C::Option, {}, {"draw","path"},
                   {"left","right","center"}, {"decorations.text"});
    }

    // ── decorations.footprints sub-keys ──
    // Verified against pgflibrarydecorations.footprints.code.tex.
    {
        const char *keys[] = {
            "foot length","stride length","foot sep","foot angle",
            nullptr};
        for (int i = 0; keys[i]; i++)
            addBuiltin(db, keys[i], C::Option, {}, {"draw","path"}, {},
                       {"decorations.footprints"});
        addBuiltin(db, "foot of", C::Option, {}, {"draw","path"},
                   {"human","bird","dog","frog","gnu"},
                   {"decorations.footprints"});
    }

    // ── decorations.markings sub-keys ──
    // Verified against pgflibrarydecorations.markings.code.tex.
    {
        addBuiltin(db, "mark", C::Option, {}, {"draw","path"}, {},
                   {"decorations.markings"});
        addBuiltin(db, "mark connection node", C::Option, {}, {"draw","path"}, {},
                   {"decorations.markings"});
        addBuiltin(db, "reset marks", C::Option, {}, {"draw","path"}, {},
                   {"decorations.markings"});
    }

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
    addBuiltin(db, "current point is local", C::Option, {}, {"draw","path","node","edge"});
    addBuiltin(db, "turn",             C::Option, {}, {"draw","path"});

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
    addBuiltin(db, "parabola height",  C::Option, {}, {"draw","path"});
    addBuiltin(db, "bend",             C::Option, {}, {"draw","path"});

    // Plot options (\draw plot ...): defined in tikz.code.tex. Note that
    // pgfplots-specific "domain"/"samples" (with value hints) live in
    // tikz_keywords_pgfplots.cpp gated to axis environments.
    addBuiltin(db, "variable",         C::Option, {}, {"draw","path"});
    addBuiltin(db, "smooth cycle",     C::Option, {}, {"draw","path"});
    addBuiltin(db, "sharp plot",       C::Option, {}, {"draw","path"});
    addBuiltin(db, "sharp cycle",      C::Option, {}, {"draw","path"});
    addBuiltin(db, "const plot",       C::Option, {}, {"draw","path"});
    addBuiltin(db, "const plot mark left",  C::Option, {}, {"draw","path"});
    addBuiltin(db, "const plot mark right", C::Option, {}, {"draw","path"});
    addBuiltin(db, "const plot mark mid",   C::Option, {}, {"draw","path"});
    addBuiltin(db, "jump mark left",   C::Option, {}, {"draw","path"});
    addBuiltin(db, "jump mark right",  C::Option, {}, {"draw","path"});
    addBuiltin(db, "jump mark mid",    C::Option, {}, {"draw","path"});
    addBuiltin(db, "ycomb",            C::Option, {}, {"draw","path"});
    addBuiltin(db, "xcomb",            C::Option, {}, {"draw","path"});
    addBuiltin(db, "polar comb",       C::Option, {}, {"draw","path"});

    // Grid step (tikz.code.tex \tikzoption{step}/{xstep}/{ystep})
    addBuiltin(db, "step",             C::Option, {}, {"draw","path"});
    addBuiltin(db, "xstep",            C::Option, {}, {"draw","path"});
    addBuiltin(db, "ystep",            C::Option, {}, {"draw","path"});

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
    addBuiltin(db, "column sep",            C::Option, {}, {"node"},
               {"1mm","2mm","3mm","5mm","1em","1cm","2cm",
                "small","large","between origins"}, {"matrix"});
    addBuiltin(db, "row sep",               C::Option, {}, {"node"},
               {"1mm","2mm","3mm","5mm","1em","1cm","2cm",
                "small","large","between origins"}, {"matrix"});
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
    // 'name path'/'name path global' assign a name to the current path; they are
    // valid on \draw/\path as well as \node (verified: a \node[...,name path=...]
    // registers the node's outline as a named path under \usetikzlibrary{through,
    // intersections}). 'name intersections' only makes sense on \path/\draw.
    addBuiltin(db, "name path",         C::Option, {}, {"draw","path","node"}, {}, {"intersections"});
    addBuiltin(db, "name path global",  C::Option, {}, {"draw","path","node"}, {}, {"intersections"});
    addBuiltin(db, "name intersections",C::Option, {}, {"draw","path"}, {}, {"intersections"});
    addBuiltin(db, "of",                C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "by",                C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "sort by",           C::Option, {}, {}, {}, {"intersections"});
    addBuiltin(db, "total",             C::Option, {}, {}, {}, {"intersections"});

    // Petri library (verified against tikzlibrarypetri.code.tex)
    // Node styles for places, transitions and tokens.
    addBuiltin(db, "place",              C::Option, {}, {"node"}, {}, {"petri"});
    addBuiltin(db, "every place",        C::Option, {}, {"node"}, {}, {"petri"});
    addBuiltin(db, "transition",         C::Option, {}, {"node"}, {}, {"petri"});
    addBuiltin(db, "every transition",   C::Option, {}, {"node"}, {}, {"petri"});
    addBuiltin(db, "token",              C::Option, {}, {"node"}, {}, {"petri"});
    addBuiltin(db, "every token",        C::Option, {}, {"node"}, {}, {"petri"});
    // Relationship (edge) style unique to petri (pre/post already exist).
    addBuiltin(db, "pre and post",       C::Option, {}, {}, {}, {"petri"});
    // Token-placement keys/options.
    addBuiltin(db, "tokens",             C::Option, {}, {"node"},
               {"1","2","3","4","5","6","7","8","9"}, {"petri"});
    addBuiltin(db, "colored tokens",     C::Option, {}, {"node"}, {}, {"petri"});
    addBuiltin(db, "structured tokens",  C::Option, {}, {"node"}, {}, {"petri"});
    addBuiltin(db, "children are tokens",C::Option, {}, {"node"}, {}, {"petri"});
    addBuiltin(db, "token distance",     C::Option, {}, {},
               {"1ex","1.5ex","2ex","3ex"}, {"petri"});

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
    // Additional shapes.multipart keys (verified against tikzlibraryshapes.multipart.code.tex)
    addBuiltin(db, "rectangle split draw splits", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});
    addBuiltin(db, "rectangle split part align", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});
    addBuiltin(db, "rectangle split part fill", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});
    addBuiltin(db, "rectangle split use custom fill", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});
    addBuiltin(db, "rectangle split empty part width", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});
    addBuiltin(db, "rectangle split empty part height", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});
    addBuiltin(db, "rectangle split empty part depth", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.multipart"});

    // shapes.symbols
    addBuiltin(db, "shape border uses incircle", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.symbols"});

    // shapes.callouts
    addBuiltin(db, "callout absolute pointer", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.callouts"});
    addBuiltin(db, "callout relative pointer", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.callouts"});

    // shapes.arrows
    addBuiltin(db, "arrow box arrows", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"shapes.arrows"});

    // Tree / graph
    addBuiltin(db, "grow",              C::Option, {"tikzpicture","scope"}, {},
               {"up","down","left","right","north","south","east","west"});
    addBuiltin(db, "grow'",             C::Option, {"tikzpicture","scope"}, {},
               {"up","down","left","right"});
    addBuiltin(db, "grow cyclic",       C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "growth function",   C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "growth parent anchor", C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "level distance",    C::Option, {"tikzpicture","scope"},
               {"0.5cm","1cm","1.5cm","2cm","3cm"});
    addBuiltin(db, "sibling distance",  C::Option, {"tikzpicture","scope"},
               {"0.5cm","1cm","1.5cm","2cm","3cm"});
    addBuiltin(db, "edge from parent",  C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "edge from parent path", C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "edge label",        C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "parent anchor",     C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "child anchor",      C::Option, {"tikzpicture","scope"});
    addBuiltin(db, "counterclockwise",  C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "clockwise",         C::Option, {"tikzpicture","scope"}, {"node"});
    addBuiltin(db, "sibling angle",     C::Option, {"tikzpicture","scope"}, {}, {}, {"trees"});

    // Mindmap
    addBuiltin(db, "mindmap",           C::Option, {"tikzpicture"});
    addBuiltin(db, "concept",           C::Option, {"tikzpicture"}, {}, {}, {"mindmap"});
    addBuiltin(db, "concept color",     C::Option, {"tikzpicture"}, {}, {}, {"mindmap"});

    // ── Chains ── (verified against tikzlibrarychains.code.tex)
    {
        const char *keys[] = {
            "start chain","continue chain","on chain","join",
            "chain default direction","start branch","continue branch",
            nullptr};
        for (int i = 0; keys[i]; i++)
            addBuiltin(db, keys[i], C::Option, {"tikzpicture","scope"}, {}, {}, {"chains"});
    }

    // ── Spy ── (verified against tikzlibraryspy.code.tex)
    {
        addBuiltin(db, "spy using outlines", C::Option, {"tikzpicture","scope"}, {}, {}, {"spy"});
        addBuiltin(db, "spy using overlays", C::Option, {"tikzpicture","scope"}, {}, {}, {"spy"});
        addBuiltin(db, "connect spies", C::Option, {"tikzpicture","scope"}, {}, {}, {"spy"});
        addBuiltin(db, "lens", C::Option, {"tikzpicture","scope"}, {}, {}, {"spy"});
        addBuiltin(db, "magnification", C::Option, {"tikzpicture","scope"}, {}, {}, {"spy"});
        addBuiltin(db, "spy connection path", C::Option, {"tikzpicture","scope"}, {}, {}, {"spy"});
    }

    // ── Through ── (verified against tikzlibrarythrough.code.tex)
    {
        addBuiltin(db, "circle through", C::Option, {"tikzpicture","scope"}, {}, {}, {"through"});
    }

    // ── Shadows (additional) ── (verified against tikzlibraryshadows.code.tex)
    {
        addBuiltin(db, "shadow scale", C::Option, {"tikzpicture","scope"}, {}, {}, {"shadows"});
        addBuiltin(db, "shadow xshift", C::Option, {"tikzpicture","scope"}, {}, {}, {"shadows"});
        addBuiltin(db, "shadow yshift", C::Option, {"tikzpicture","scope"}, {}, {}, {"shadows"});
        addBuiltin(db, "copy shadow", C::Option, {"tikzpicture","scope"}, {}, {}, {"shadows"});
        addBuiltin(db, "double copy shadow", C::Option, {"tikzpicture","scope"}, {}, {}, {"shadows"});
        addBuiltin(db, "circular glow", C::Option, {"tikzpicture","scope"}, {}, {}, {"shadows"});
    }

    // ── Fit library ── (verified against tikzlibraryfit.code.tex)
    {
        addBuiltin(db, "rotate fit", C::Option, {"tikzpicture","scope"}, {}, {}, {"fit"});
        addBuiltin(db, "every fit", C::Option, {"tikzpicture","scope"}, {}, {}, {"fit"});
    }

    // ── patterns.meta ── (verified against tikzlibrarypatterns.meta.code.tex)
    {
        addBuiltin(db, "patterns/tile size", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"patterns.meta"});
        addBuiltin(db, "patterns/bounding box", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"patterns.meta"});
        addBuiltin(db, "patterns/top right", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"patterns.meta"});
        addBuiltin(db, "patterns/bottom left", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"patterns.meta"});
        addBuiltin(db, "patterns/tile transformation", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"patterns.meta"});
        addBuiltin(db, "patterns/infer tile bounding box", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"patterns.meta"});
    }

    // ── perspective / 3d ── (verified against tikzlibraryperspective.code.tex)
    {
        addBuiltin(db, "3d view", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"perspective"});
    }

    // ── shapes.gates.logic ── (verified against tikzlibraryshapes.gates.logic.*.code.tex)
    {
        addBuiltin(db, "use US style logic gates", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"shapes.gates.logic.US"});
        addBuiltin(db, "use CDH style logic gates", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"shapes.gates.logic.US"});
        addBuiltin(db, "use IEC style logic gates", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"shapes.gates.logic.IEC"});

        const char *gateStyles[] = {
            "and gate","nand gate","or gate","nor gate",
            "xor gate","xnor gate","not gate","buffer gate",
            nullptr};
        for (int i = 0; gateStyles[i]; i++)
            addBuiltin(db, gateStyles[i], C::Option, {"tikzpicture","scope"}, {"node"},
                       {}, {"shapes.gates.logic.US"});

        const char *gateSymbols[] = {
            "and gate symbol","nand gate symbol","or gate symbol",
            "nor gate symbol","xor gate symbol","xnor gate symbol",
            "not gate symbol","buffer gate symbol",
            nullptr};
        for (int i = 0; gateSymbols[i]; i++)
            addBuiltin(db, gateSymbols[i], C::Option, {"tikzpicture","scope"}, {"node"},
                       {}, {"shapes.gates.logic.IEC"});
    }

    // ── Mindmap (additional) ── (verified against tikzlibrarymindmap.code.tex)
    {
        addBuiltin(db, "root concept", C::Option, {"tikzpicture"}, {}, {},
                   {"mindmap"});
        addBuiltin(db, "concept connection", C::Option, {"tikzpicture"}, {}, {},
                   {"mindmap"});
        addBuiltin(db, "extra concept", C::Option, {"tikzpicture"}, {}, {},
                   {"mindmap"});
        addBuiltin(db, "annotation", C::Option, {"tikzpicture"}, {}, {},
                   {"mindmap"});
        addBuiltin(db, "circle connection bar", C::Option, {"tikzpicture"}, {}, {},
                   {"mindmap"});
    }

    // ── Animations ── (verified against tikzlibraryanimations.code.tex)
    {
        addBuiltin(db, "animate", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"animations"});
        addBuiltin(db, "make snapshot of", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"animations"});
    }

    // ── Lindenmayer systems ── (verified against tikzlibrarylindenmayersystems.code.tex)
    {
        addBuiltin(db, "lindenmayer system", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"lindenmayersystems"});
    }

    // ── Folding ── (verified against tikzlibraryfolding.code.tex)
    {
        addBuiltin(db, "folding line length", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"folding"});
        addBuiltin(db, "face 1", C::Option, {"tikzpicture","scope"}, {}, {},
                   {"folding"});
    }

    // ── Turtle graphics ── (verified against tikzlibraryturtle.code.tex)
    {
        addBuiltin(db, "turtle/distance", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"turtle"});
        addBuiltin(db, "turtle/forward", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"turtle"});
        addBuiltin(db, "turtle/back", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"turtle"});
        addBuiltin(db, "turtle/left", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"turtle"});
        addBuiltin(db, "turtle/right", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"turtle"});
        addBuiltin(db, "turtle/how", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"turtle"});
        addBuiltin(db, "turtle/home", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"turtle"});
    }

    // ── RDF ── (verified against tikzlibraryrdf.code.tex)
    {
        addBuiltin(db, "rdf engine", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"rdf"});
        addBuiltin(db, "subject", C::Option, {"tikzpicture","scope"}, {},
                   {}, {"rdf"});
    }

    // ── Views ── (verified against tikzlibraryviews.code.tex)
    {
        addBuiltin(db, "meet", C::Option, {"tikzpicture","scope"}, {}, {},
                   {"views"});
    }

    // Automata (verified against tikzlibraryautomata.code.tex) — gated by automata
    {
        const char *ks[] = {
            "state","state with output","state without output","every state",
            "accepting","accepting by arrow","accepting by double",
            "accepting above","accepting below","accepting left","accepting right",
            "accepting text","accepting where","accepting distance",
            "initial","initial by arrow","initial by diamond",
            "initial above","initial below","initial left","initial right",
            "initial text","initial where","initial distance",
            nullptr};
        for (int i = 0; ks[i]; i++)
            addBuiltin(db, ks[i], C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"automata"});
    }
    // Loop styles (topaths, always available)
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
    // 'to path' is a general TikZ option (\tikzoption{to path}), commonly set in
    // \tikzset{}/styles (no command context) as well as directly on \draw/\path
    // via the 'to'/'edge' operations — so keep it unrestricted rather than gated
    // to draw/path. Its value is a path expression that may reference the
    // \tikztostart, \tikztotarget and \tikztonodes macros.
    addBuiltin(db, "to path",           C::Option, {"tikzpicture","scope"});

    // ── Additional TikZ library option keys (all verified against the
    //    respective tikzlibrary*.code.tex sources) ──

    // backgrounds
    {
        const char *ks[] = {
            "framed","gridded","tight background","loose background",
            "show background grid","background grid",
            "show background top","show background bottom",
            "show background left","show background right",
            "background top","background bottom","background left","background right",
            "inner frame sep","inner frame xsep","inner frame ysep",
            "outer frame sep","outer frame xsep","outer frame ysep",
            nullptr};
        for (int i = 0; ks[i]; i++)
            addBuiltin(db, ks[i], C::Option, {"tikzpicture","scope"}, {}, {}, {"backgrounds"});
    }

    // positioning
    addBuiltin(db, "base left",  C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"positioning"});
    addBuiltin(db, "base right", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"positioning"});
    addBuiltin(db, "mid left",   C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"positioning"});
    addBuiltin(db, "mid right",  C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"positioning"});

    // topaths (always available with tikz) — to[...] path keys
    addBuiltin(db, "relative",          C::Option, {}, {"draw","path"});
    addBuiltin(db, "in control",        C::Option, {}, {"draw","path"});
    addBuiltin(db, "out control",       C::Option, {}, {"draw","path"});
    addBuiltin(db, "in min distance",   C::Option, {}, {"draw","path"});
    addBuiltin(db, "in max distance",   C::Option, {}, {"draw","path"});
    addBuiltin(db, "out min distance",  C::Option, {}, {"draw","path"});
    addBuiltin(db, "out max distance",  C::Option, {}, {"draw","path"});
    addBuiltin(db, "min distance",      C::Option, {}, {"draw","path"});
    addBuiltin(db, "max distance",      C::Option, {}, {"draw","path"});
    addBuiltin(db, "loop",              C::Option, {"tikzpicture","scope"});

    // trees
    {
        const char *ks[] = {
            "edge from parent fork down","edge from parent fork up",
            "edge from parent fork left","edge from parent fork right",
            "grow via three points","clockwise from","counterclockwise from",
            nullptr};
        for (int i = 0; ks[i]; i++)
            addBuiltin(db, ks[i], C::Option, {"tikzpicture","scope"}, {}, {}, {"trees"});
    }

    // er (entity-relationship)
    addBuiltin(db, "entity",        C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"er"});
    addBuiltin(db, "relationship",  C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"er"});
    addBuiltin(db, "attribute",     C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"er"});
    addBuiltin(db, "key attribute", C::Option, {"tikzpicture","scope"}, {"node"}, {}, {"er"});

    // matrix
    addBuiltin(db, "above delimiter",     C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "below delimiter",     C::Option, {}, {"node"}, {}, {"matrix"});
    addBuiltin(db, "nodes in empty cells",C::Option, {}, {"node"}, {}, {"matrix"});

    // shadings — corner colour keys
    addBuiltin(db, "upper left",  C::Option, {}, {"draw","path","fill","shade","shadedraw"}, {}, {"shadings"});
    addBuiltin(db, "upper right", C::Option, {}, {"draw","path","fill","shade","shadedraw"}, {}, {"shadings"});
    addBuiltin(db, "lower left",  C::Option, {}, {"draw","path","fill","shade","shadedraw"}, {}, {"shadings"});
    addBuiltin(db, "lower right", C::Option, {}, {"draw","path","fill","shade","shadedraw"}, {}, {"shadings"});

    // mindmap (extends existing mindmap keys)
    {
        const char *ks[] = {
            "small mindmap","large mindmap","huge mindmap",
            "level 1 concept","level 2 concept","level 3 concept","level 4 concept",
            "level 1","level 2","level 3","level 4",
            "circle connection bar switch color",
            nullptr};
        for (int i = 0; ks[i]; i++)
            addBuiltin(db, ks[i], C::Option, {"tikzpicture","scope"}, {}, {}, {"mindmap"});
    }

    // 3d — canvas plane keys
    {
        const char *ks[] = {
            "canvas is xz plane at y","canvas is yz plane at x",
            "canvas is yx plane at z","canvas is zx plane at y","canvas is zy plane at x",
            "plane origin","plane x","plane y",
            nullptr};
        for (int i = 0; ks[i]; i++)
            addBuiltin(db, ks[i], C::Option, {"tikzpicture","scope"}, {}, {}, {"3d"});
    }

    // quotes
    addBuiltin(db, "node quotes mean",   C::Option, {"tikzpicture","scope"}, {}, {}, {"quotes"});
    addBuiltin(db, "edge quotes mean",   C::Option, {"tikzpicture","scope"}, {}, {}, {"quotes"});
    addBuiltin(db, "pic quotes mean",    C::Option, {"tikzpicture","scope"}, {}, {}, {"quotes"});
    addBuiltin(db, "quotes mean label",  C::Option, {"tikzpicture","scope"}, {}, {}, {"quotes"});
    addBuiltin(db, "quotes mean pin",    C::Option, {"tikzpicture","scope"}, {}, {}, {"quotes"});
    addBuiltin(db, "direction shorthands", C::Option, {"tikzpicture","scope"}, {}, {}, {"quotes"});

    // decorations.text
    {
        const char *ks[] = {
            "text effects","group letters","group letters into words",
            "reverse text","repeat text","replace characters","style characters",
            "scale text to path","fit text to path","path from text",
            "path from text angle","word separator","character command",
            nullptr};
        for (int i = 0; ks[i]; i++)
            addBuiltin(db, ks[i], C::Option, {}, {"draw","path"}, {}, {"decorations.text"});
    }

    // calendar
    {
        const char *ks[] = {
            "dates","day list downward","day list upward","day list left","day list right",
            "week list","month list","day text","month text","year text",
            "day code","month code","year code","day xshift","day yshift",
            "month xshift","month yshift",
            "month label above centered","month label above left","month label above right",
            "month label below centered","month label below left",
            "month label left","month label left vertical",
            "month label right","month label right vertical",
            nullptr};
        for (int i = 0; ks[i]; i++)
            addBuiltin(db, ks[i], C::Option, {"tikzpicture","scope"}, {}, {}, {"calendar"});
    }

    // turtle
    addBuiltin(db, "fd", C::Option, {"tikzpicture","scope"}, {}, {}, {"turtle"});
    addBuiltin(db, "bk", C::Option, {"tikzpicture","scope"}, {}, {}, {"turtle"});
    addBuiltin(db, "lt", C::Option, {"tikzpicture","scope"}, {}, {}, {"turtle"});
    addBuiltin(db, "rt", C::Option, {"tikzpicture","scope"}, {}, {}, {"turtle"});

    // lindenmayersystems
    addBuiltin(db, "l-system", C::Option, {}, {"draw","path"}, {}, {"lindenmayersystems"});
    addBuiltin(db, "axiom",    C::Option, {}, {}, {}, {"lindenmayersystems"});
    addBuiltin(db, "rule set", C::Option, {}, {}, {}, {"lindenmayersystems"});

    // intersections (extends existing)
    addBuiltin(db, "name path local", C::Option, {}, {"draw","path","node"}, {}, {"intersections"});

    // perspective
    addBuiltin(db, "isometric view", C::Option, {"tikzpicture","scope"}, {}, {}, {"perspective"});

    // spy
    addBuiltin(db, "spy scope", C::Option, {"tikzpicture","scope"}, {}, {}, {"spy"});

    // views
    addBuiltin(db, "slice", C::Option, {"tikzpicture","scope"}, {}, {}, {"views"});

    // shadows
    addBuiltin(db, "general shadow", C::Option, {}, {}, {}, {"shadows"});

    // shapes.gates.logic (IEC/US) — symbol styling keys
    addBuiltin(db, "logic gate symbol align", C::Option, {"tikzpicture","scope"}, {"node"}, {},
               {"shapes.gates.logic.IEC"});
    addBuiltin(db, "logic gate symbol color", C::Option, {"tikzpicture","scope"}, {"node"}, {},
               {"shapes.gates.logic.IEC"});

    // rdf
    {
        const char *ks[] = {
            "has type","has as member","is a bag","is a container",
            "is a sequence","is an alternative","scope is new context",
            nullptr};
        for (int i = 0; ks[i]; i++)
            addBuiltin(db, ks[i], C::Option, {"tikzpicture","scope"}, {}, {}, {"rdf"});
    }

    // Coordinate systems
    addBuiltin(db, "x",                 C::Option, {}, {});
    addBuiltin(db, "y",                 C::Option, {}, {});
    addBuiltin(db, "z",                 C::Option, {}, {});
}

} // namespace TikzKeywords
