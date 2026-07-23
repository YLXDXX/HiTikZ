#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <QMap>
#include <QHash>

struct Snippet {
    QString id;
    QString name;
    QString description;
    QString category;
    QStringList tags;
    QString templateId;
    QString packages;
    QString tikzLibraries;
    QString compileCommand;
    QString code;
    bool isPreset = false;
    double sortOrder = 0.0;
};
Q_DECLARE_METATYPE(Snippet)

struct SearchResult {
    Snippet snippet;
    int score;
};

class SnippetManager : public QObject {
    Q_OBJECT
public:
    explicit SnippetManager(QObject *parent = nullptr);

    QString getBasePath() const;
    QString getPresetPath() const;
    QString getResourcePresetPath() const;

    Snippet loadSnippet(const QString &id);
    Snippet loadPreset(const QString &id);
    bool saveSnippet(const Snippet &s);
    bool deleteSnippet(const QString &id);
    QList<Snippet> getAllSnippets(bool loadCode = false) const;
    QList<Snippet> getAllPresets(bool loadCode = false) const;
    QString createSnippet(const QString &name, const QString &category);
    bool snippetExists(const QString &id) const;
    bool isPresetId(const QString &id) const;

    bool updateSnippetCategory(const QString &id, const QString &newCategory);
    int renameCategory(const QString &oldCategory, const QString &newCategory);
    int deleteCategory(const QString &category);

    static void copyPresetsFromResources(const QString &resourceDir, const QString &destDir);
    static int fuzzyMatchScore(const QString &query, const QString &target);

    // Returns true when a snippet whose category is `snippetCategory` should be
    // shown under the category-tree node `filterCategory`. A node matches its own
    // category and any descendant (prefix followed by '/'), but NOT sibling
    // categories that merely share a textual prefix (e.g. "数学" vs "数学分析").
    static bool categoryMatches(const QString &snippetCategory,
                                const QString &filterCategory);
    QList<SearchResult> searchSnippets(const QString &query, bool includePresets = true) const;
    QStringList getAllCategories(bool includePresets = true) const;
    QMap<QString, int> getCategoryCounts(bool includePresets = true) const;
    int getUncategorizedCount(bool includePresets = true) const;

    bool exportSnippetZip(const QString &id, const QString &zipPath);
    bool exportSnippetsZip(const QStringList &ids, const QString &zipPath);
    QStringList importSnippetsZip(const QString &zipPath);

    bool batchUpdateCategory(const QStringList &ids, const QString &newCategory);
    int batchDeleteSnippets(const QStringList &ids);

    void reorderSnippets(const QStringList &orderedIds);
    static QString categoryOrderFile();
    QStringList loadCategoryOrder() const;
    void saveCategoryOrder(const QStringList &order);

    void invalidateCaches();

signals:
    void snippetCreated(const QString &id);
    void snippetDeleted(const QString &id);
    void snippetModified(const QString &id);
    void categoriesChanged();

private:
    QString basePath;
    QString presetPath;
    mutable QSet<QString> presetIdsCache;
    mutable bool presetIdsCached = false;

    QString getSnippetPath(const QString &id) const;
    QString getPresetSnippetPath(const QString &id) const;
    QJsonObject snippetToJson(const Snippet &s) const;
    Snippet jsonToSnippet(const QJsonObject &obj) const;
    void ensurePresetIdsCached() const;
    void ensureCountsCached() const;
    void invalidateCachesLight() const;
    Snippet loadMetaFromDir(const QString &dirPath) const;
    void loadCodeForSnippet(const QString &dirPath, Snippet &s) const;
    mutable QMap<QString, int> m_cachedCategoryCounts;
    mutable bool m_countsCached = false;
    struct SearchIndex {
        QHash<QString, QSet<int>> bigramIndex;
        QStringList allTexts;
        QStringList allIds;
        bool built = false;
    };
    mutable SearchIndex m_searchIndex;
    void ensureSearchIndexBuilt() const;
    void addSnippetToSearchIndex(const Snippet &s) const;
    void removeSnippetFromSearchIndex(const QString &id) const;
};
