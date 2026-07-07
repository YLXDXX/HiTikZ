#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <QSet>
#include <QHash>
#include <QPair>

namespace TikzKeywords {

enum class Category {
    Option,
    Anchor,
    Shape,
    Color,
    Arrow,
    LineWidth,
    LineType,
    Pattern,
    Decoration,
    Library,
    Command,
    Environment,
    MathFunction,
    Handler,
    PGFKeyPath,
    UserDefined
};

struct TikzKeyword {
    QString name;
    Category category;
    QSet<QString> environments;
    QSet<QString> commands;
    QSet<QString> requiredLibs;
    QStringList valueHints;
    QString doc;
};

class TikzKeywordDB {
public:
    static TikzKeywordDB &instance();

    QVector<const TikzKeyword*> filter(
        const QString &contextEnv,
        const QString &contextCmd,
        const QSet<QString> &activeLibs,
        Category cat = Category::Option,
        bool includeUserDefined = true) const;

    const TikzKeyword *find(const QString &name, Category cat) const;

    QStringList names(Category cat) const;
    QStringList allOptionNames() const;
    QStringList allAnchorNames() const;
    QStringList allColorNames() const;
    QStringList allArrowNames() const;
    QStringList allLineTypeNames() const;
    QStringList allLineWidthNames() const;
    QStringList allCommandNames() const;
    QStringList allEnvNames() const;
    QStringList allLibNames() const;
    QStringList allHandlerNames() const;
    QStringList allMathFuncNames() const;
    QStringList allCompletableWords() const;
    QStringList allPatternNames() const;
    QStringList allDecorationNames() const;

    QVector<QPair<QString, QStringList>> allValueHints() const;

    QStringList valueHintsFor(const QString &keyName) const;

    void registerUserDefined(const QString &name, Category cat,
                             const QString &doc = QString());
    void clearUserDefined();

private:
    TikzKeywordDB();
    void initBuiltins();

    QVector<TikzKeyword> m_builtin;
    QVector<TikzKeyword> m_userDefined;
    QHash<QString, int> m_nameIndex;

    QString makeKey(const QString &name, Category cat) const;
};

} // namespace TikzKeywords
