#pragma once
#include <QStringList>
#include <QVector>
#include <QPair>
#include <QSet>
#include "tikz_keywords.h"

namespace TikzWords {

inline const QStringList tikzCommands() {
    return TikzKeywords::TikzKeywordDB::instance().allCommandNames();
}
inline const QStringList tikzEnvironments() {
    return TikzKeywords::TikzKeywordDB::instance().allEnvNames();
}
inline const QStringList tikzOptions() {
    return TikzKeywords::TikzKeywordDB::instance().allOptionNames();
}
inline const QStringList tikzAnchors() {
    return TikzKeywords::TikzKeywordDB::instance().allAnchorNames();
}
inline const QStringList tikzKeyHandlers() {
    return TikzKeywords::TikzKeywordDB::instance().allHandlerNames();
}
inline const QStringList tikzColors() {
    return TikzKeywords::TikzKeywordDB::instance().allColorNames();
}
inline const QStringList tikzArrows() {
    return TikzKeywords::TikzKeywordDB::instance().allArrowNames();
}
inline const QStringList tikzLineTypes() {
    return TikzKeywords::TikzKeywordDB::instance().allLineTypeNames();
}
inline const QStringList tikzLineWidths() {
    return TikzKeywords::TikzKeywordDB::instance().allLineWidthNames();
}
inline const QStringList tikzMathFunctions() {
    return TikzKeywords::TikzKeywordDB::instance().allMathFuncNames();
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
    return TikzKeywords::TikzKeywordDB::instance().allValueHints();
}

inline QStringList allCompletableWords() {
    return TikzKeywords::TikzKeywordDB::instance().allCompletableWords();
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
        "/tikz/circuitikz",
        "/tikz/bipoles", "/tikz/tripoles", "/tikz/quadpoles",
        "/tikz/bipoles/generic", "/tikz/bipoles/resistor",
        "/tikz/bipoles/capacitor", "/tikz/bipoles/inductor",
        "/tikz/bipoles/diode", "/tikz/bipoles/source",
        "/tikz/bipoles/meter", "/tikz/bipoles/switch",
        "/tikz/bipoles/mechanical", "/tikz/bipoles/transistor",
        "/tikz/tripoles/op amp", "/tikz/tripoles/bjt",
        "/tikz/tripoles/igbt", "/tikz/tripoles/mos",
        "/tikz/tripoles/jfet", "/tikz/tripoles/tube",
        "/tikz/quadpoles/transformer", "/tikz/quadpoles/fourport",
        "/tikz/quadpoles/gyrator", "/tikz/quadpoles/coupling",
    };
    return list;
}

} // namespace TikzWords
