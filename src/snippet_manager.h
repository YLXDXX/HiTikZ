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
    // Image files stored alongside the snippet (e.g. "a.png", "b.pdf").
    QStringList images;
    // File name of the "复制链接" picture link in the shared link directory
    // (e.g. "0001.pdf"); empty when no link has been created.
    QString linkedPdf;
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
    void addCategory(const QString &category);

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

    // ── Image management ──────────────────────────────────────────────────
    // Images are stored inside the snippet's own directory with letter
    // sequence names (a.png, b.png, ..., z.png, aa.png, ...). The `images`
    // field of meta.json records the file names.

    // File-name extensions accepted as snippet images.
    static QStringList supportedImageExtensions();
    static bool isSupportedImageFile(const QString &fileName);
    // 0 → "a", 1 → "b", ..., 25 → "z", 26 → "aa", 27 → "ab", ...
    static QString imageStemForIndex(int index);
    // Directory where a snippet's images live (preset-aware).
    QString getSnippetImageDir(const QString &id) const;
    // Full paths of the images listed in the snippet's meta.json that still
    // exist on disk.
    QStringList getSnippetImagePaths(const QString &id);

    // Copy `srcFilePath` into the snippet's directory using the next free
    // letter name (keeping the source's extension). Returns the new file name
    // ("a.png", ...) or an empty string on failure.
    QString addImageToSnippet(const QString &id, const QString &srcFilePath);
    // Replace an existing image, keeping its letter stem but adopting the
    // source file's extension. Returns the new file name or empty on failure.
    QString replaceSnippetImage(const QString &id, const QString &imageName,
                                const QString &srcFilePath);
    bool removeSnippetImage(const QString &id, const QString &imageName);
    // Copy the image files listed in `srcId`'s meta.json into `dstId`'s
    // directory (used when duplicating a snippet so code references keep
    // working). Returns false if any file copy failed.
    bool copySnippetImageFiles(const QString &srcId, const QString &dstId);

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

    QString categoryListFile() const;
    QStringList loadAllPersistedCategories() const;
    void savePersistedCategories(const QStringList &cats);
    void removeCategoryFromPersisted(const QString &category);
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
