#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>

struct Snippet {
    QString id;
    QString name;
    QString description;
    QString category;
    QStringList tags;
    QString templateId;
    QString code;
    bool isPreset = false;
};

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
    QList<Snippet> getAllSnippets() const;
    QList<Snippet> getAllPresets() const;
    QString createSnippet(const QString &name, const QString &category);
    bool snippetExists(const QString &id) const;
    bool isPresetId(const QString &id) const;

    static void copyPresetsFromResources(const QString &resourceDir, const QString &destDir);
    static int fuzzyMatchScore(const QString &query, const QString &target);
    QList<SearchResult> searchSnippets(const QString &query, bool includePresets = true) const;
    QStringList getAllCategories(bool includePresets = true) const;

signals:
    void snippetCreated(const QString &id);
    void snippetDeleted(const QString &id);
    void snippetModified(const QString &id);

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
};
