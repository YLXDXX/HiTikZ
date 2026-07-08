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
inline const QStringList tikzLibraries() {
    return TikzKeywords::TikzKeywordDB::instance().allLibNames();
}
inline const QStringList tikzPatternNames() {
    return TikzKeywords::TikzKeywordDB::instance().allPatternNames();
}
inline const QStringList tikzDecorationNames() {
    return TikzKeywords::TikzKeywordDB::instance().allDecorationNames();
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

inline const QStringList pgfKeysPathCommands() {
    return TikzKeywords::TikzKeywordDB::instance().names(TikzKeywords::Category::PGFKeyPath);
}

} // namespace TikzWords
