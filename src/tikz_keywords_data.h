#pragma once
#include "tikz_keywords.h"

namespace TikzKeywords {

using Vec = QVector<TikzKeyword>;
using C = Category;

// ── Shared helpers (used by all data registration files) ──
void addBuiltin(Vec &db, const char *name, Category cat,
                std::initializer_list<const char *> envs = {},
                std::initializer_list<const char *> cmds = {},
                std::initializer_list<const char *> vals = {},
                std::initializer_list<const char *> libs = {},
                const char *doc = nullptr);

// ── Per-category registration functions ──
void registerColors(Vec &db);
void registerLineWidths(Vec &db);
void registerLineTypes(Vec &db);
void registerArrows(Vec &db);
void registerShapes(Vec &db);
void registerPatterns(Vec &db);
void registerDecorations(Vec &db);
void registerAnchors(Vec &db);
void registerHandlers(Vec &db);
void registerPgfKeyPaths(Vec &db);
void registerLibraries(Vec &db);
void registerEnvironments(Vec &db);
void registerCommands(Vec &db);
void registerMathFunctions(Vec &db);
void registerGeneralOptions(Vec &db);
void registerPgfplotsAndShapes(Vec &db);
void registerExtended(Vec &db);

// ── Unified registration entry point ──
void registerAllBuiltins(Vec &db);

} // namespace TikzKeywords
