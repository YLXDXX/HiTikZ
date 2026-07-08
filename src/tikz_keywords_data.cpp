#include "tikz_keywords_data.h"

namespace TikzKeywords {

void addBuiltin(Vec &db, const char *name, Category cat,
                std::initializer_list<const char *> envs,
                std::initializer_list<const char *> cmds,
                std::initializer_list<const char *> vals,
                std::initializer_list<const char *> libs,
                const char *doc)
{
    TikzKeyword kw;
    kw.name = QString::fromUtf8(name);
    kw.category = cat;
    for (auto &e : envs) kw.environments.insert(QString::fromUtf8(e));
    for (auto &c : cmds) kw.commands.insert(QString::fromUtf8(c));
    for (auto &v : vals) kw.valueHints.append(QString::fromUtf8(v));
    for (auto &l : libs) kw.requiredLibs.insert(QString::fromUtf8(l));
    kw.doc = doc ? QString::fromUtf8(doc) : QString();
    db.append(kw);
}

void addBipolePrefixes(Vec &db, const char *bipole,
                       std::initializer_list<const char *> envs)
{
    static const char *prefixes[] = {
        "p","v","i","ld","s","q","o","vq","iq","qq", nullptr
    };
    for (int p = 0; prefixes[p]; p++) {
        QByteArray name = QByteArray(prefixes[p]) + bipole;
        addBuiltin(db, name.constData(), C::Command, envs,
                   {"draw","path","to"}, {}, {}, nullptr);
    }
}

void addCtikzShapes(Vec &db, std::initializer_list<const char *> names,
                    std::initializer_list<const char *> envs)
{
    for (auto &n : names) {
        QByteArray shapeName = QByteArray(n) + "shape";
        addBuiltin(db, shapeName.constData(), C::Shape, envs, {"node"});
    }
}

void addCtikzShape(Vec &db, const char *shapeName,
                   std::initializer_list<const char *> envs)
{
    addBuiltin(db, shapeName, C::Shape, envs, {"node"});
}

void registerAllBuiltins(Vec &db)
{
    registerColors(db);
    registerLineWidths(db);
    registerLineTypes(db);
    registerArrows(db);
    registerShapes(db);
    registerPatterns(db);
    registerDecorations(db);
    registerAnchors(db);
    registerHandlers(db);
    registerPgfKeyPaths(db);
    registerLibraries(db);
    registerEnvironments(db);
    registerCommands(db);
    registerMathFunctions(db);
    registerGeneralOptions(db);
    registerPgfplotsAndShapes(db);
    registerExtended(db);
}

} // namespace TikzKeywords
