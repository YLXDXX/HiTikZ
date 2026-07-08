#include "tikz_keywords.h"
#include "tikz_keywords_data.h"

namespace TikzKeywords {

void TikzKeywordDB::initBuiltins()
{
    Vec &db = m_builtin;
    registerAllBuiltins(db);

    // Build name index
    for (int i = 0; i < db.size(); i++) {
        m_nameIndex[makeKey(db[i].name, db[i].category)] = i;
    }
}

TikzKeywordDB::TikzKeywordDB()
{
    initBuiltins();
}

TikzKeywordDB &TikzKeywordDB::instance()
{
    static TikzKeywordDB db;
    return db;
}

QString TikzKeywordDB::makeKey(const QString &name, Category cat) const
{
    return name.toLower() + QChar('|') + QString::number(static_cast<int>(cat));
}

QVector<const TikzKeyword*> TikzKeywordDB::filter(
    const QString &contextEnv,
    const QString &contextCmd,
    const QSet<QString> &activeLibs,
    Category cat, bool includeUserDefined) const
{
    QVector<const TikzKeyword*> result;
    QString envLower = contextEnv.toLower();
    QString cmdLower = contextCmd.toLower();

    auto matches = [&](const TikzKeyword &kw) -> bool {
        if (kw.category != cat) return false;
        if (!kw.environments.isEmpty() && !kw.environments.contains(envLower)
            && !kw.environments.contains("tikzpicture") && !kw.environments.contains("scope"))
            return false;
        if (!kw.commands.isEmpty() && !cmdLower.isEmpty()) {
            bool cmdMatch = kw.commands.contains(cmdLower);
            if (!cmdMatch) {
                static const QSet<QString> pathCmds = {"draw","path","fill","filldraw","shade","shadedraw","edge","to"};
                static const QSet<QString> nodeCmds = {"node","pic"};
                bool isPathCmd = pathCmds.contains(cmdLower);
                bool hasNodeCmd = kw.commands.intersects(nodeCmds);
                if (!(isPathCmd && hasNodeCmd))
                    return false;
            }
        }
        if (!kw.requiredLibs.isEmpty()) {
            bool found = false;
            for (const QString &lib : kw.requiredLibs) {
                if (activeLibs.contains(lib)) { found = true; break; }
            }
            if (!found) return false;
        }
        return true;
    };

    for (const auto &kw : m_builtin) {
        if (matches(kw)) result.append(&kw);
    }
    if (includeUserDefined) {
        for (const auto &kw : m_userDefined) {
            if (matches(kw)) result.append(&kw);
        }
    }
    return result;
}

const TikzKeyword *TikzKeywordDB::find(const QString &name, Category cat) const
{
    QString key = makeKey(name, cat);
    if (m_nameIndex.contains(key)) {
        return &m_builtin[m_nameIndex[key]];
    }
    for (const auto &kw : m_userDefined) {
        if (kw.name.compare(name, Qt::CaseInsensitive) == 0 && kw.category == cat)
            return &kw;
    }
    return nullptr;
}

QStringList TikzKeywordDB::names(Category cat) const
{
    QStringList result;
    for (const auto &kw : m_builtin)
        if (kw.category == cat) result << kw.name;
    for (const auto &kw : m_userDefined)
        if (kw.category == cat) result << kw.name;
    result.removeDuplicates();
    result.sort(Qt::CaseInsensitive);
    return result;
}

QStringList TikzKeywordDB::allOptionNames() const
{
    return names(Category::Option)
        + names(Category::Color)
        + names(Category::LineWidth)
        + names(Category::LineType)
        + names(Category::Arrow)
        + names(Category::Pattern)
        + names(Category::Decoration)
        + names(Category::Shape);
}

QStringList TikzKeywordDB::allAnchorNames() const { return names(Category::Anchor); }
QStringList TikzKeywordDB::allColorNames() const { return names(Category::Color); }
QStringList TikzKeywordDB::allArrowNames() const { return names(Category::Arrow); }
QStringList TikzKeywordDB::allLineTypeNames() const { return names(Category::LineType); }
QStringList TikzKeywordDB::allLineWidthNames() const { return names(Category::LineWidth); }
QStringList TikzKeywordDB::allCommandNames() const { return names(Category::Command); }
QStringList TikzKeywordDB::allEnvNames() const { return names(Category::Environment); }
QStringList TikzKeywordDB::allLibNames() const { return names(Category::Library); }
QStringList TikzKeywordDB::allHandlerNames() const { return names(Category::Handler); }
QStringList TikzKeywordDB::allMathFuncNames() const { return names(Category::MathFunction); }
QStringList TikzKeywordDB::allPatternNames() const { return names(Category::Pattern); }
QStringList TikzKeywordDB::allDecorationNames() const { return names(Category::Decoration); }

QStringList TikzKeywordDB::allCompletableWords() const
{
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
    appendUnique(allCommandNames());
    appendUnique(allOptionNames());
    appendUnique(allAnchorNames());
    appendUnique(allColorNames());
    appendUnique(allArrowNames());
    appendUnique(allLineTypeNames());
    appendUnique(allLineWidthNames());
    appendUnique(allMathFuncNames());
    result.sort(Qt::CaseInsensitive);
    return result;
}

QVector<QPair<QString, QStringList>> TikzKeywordDB::allValueHints() const
{
    QVector<QPair<QString, QStringList>> hints;
    QSet<QString> seenKeys;
    for (const auto &kw : m_builtin) {
        if (!kw.valueHints.isEmpty() && !seenKeys.contains(kw.name.toLower())) {
            seenKeys.insert(kw.name.toLower());
            hints.append({kw.name, kw.valueHints});
        }
    }
    // Add common value hints for color and arrow keys
    QStringList colorHints = allColorNames();
    QStringList arrowHints = allArrowNames();
    QStringList lineWidthHints = names(Category::LineWidth);
    auto addHint = [&](const QString &key, const QStringList &vals, bool &added) {
        if (seenKeys.contains(key.toLower())) return;
        seenKeys.insert(key.toLower());
        hints.append({key, vals});
    };
    bool dummy = false;
    addHint("color", colorHints, dummy);
    addHint("draw", colorHints, dummy);
    addHint("fill", colorHints, dummy);
    addHint("left color", colorHints, dummy);
    addHint("right color", colorHints, dummy);
    addHint("middle color", colorHints, dummy);
    addHint("top color", colorHints, dummy);
    addHint("bottom color", colorHints, dummy);
    addHint("inner color", colorHints, dummy);
    addHint("outer color", colorHints, dummy);
    addHint("ball color", colorHints, dummy);
    addHint("pattern color", colorHints, dummy);
    addHint("concept color", colorHints, dummy);
    addHint("arrow", arrowHints, dummy);
    addHint(">", arrowHints, dummy);
    addHint(">=", arrowHints, dummy);
    addHint("-", arrowHints, dummy);
    addHint("line width", {"0.2pt","0.4pt","0.5pt","0.6pt","0.8pt","1pt","1.2pt","1.5pt","2pt","2.5pt","3pt","4pt","5pt","6pt","8pt","10pt"}, dummy);
    return hints;
}

QStringList TikzKeywordDB::valueHintsFor(const QString &keyName) const
{
    for (const auto &kw : m_builtin) {
        if (kw.name.compare(keyName, Qt::CaseInsensitive) == 0 && !kw.valueHints.isEmpty())
            return kw.valueHints;
    }
    for (const auto &pair : allValueHints()) {
        if (pair.first.compare(keyName, Qt::CaseInsensitive) == 0)
            return pair.second;
    }
    return {};
}

void TikzKeywordDB::registerUserDefined(const QString &name, Category cat,
                                         const QString &doc)
{
    // Avoid duplicates
    for (auto &kw : m_userDefined) {
        if (kw.name == name && kw.category == cat) {
            kw.doc = doc;
            return;
        }
    }
    TikzKeyword kw;
    kw.name = name;
    kw.category = cat;
    kw.doc = doc;
    m_userDefined.append(kw);
}

void TikzKeywordDB::clearUserDefined()
{
    m_userDefined.clear();
}

} // namespace TikzKeywords
