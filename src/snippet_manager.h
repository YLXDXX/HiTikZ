#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

struct Snippet {
    QString id;
    QString name;
    QString description;
    QString category;
    QStringList tags;
    QString templateId;
    QString code;
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
    Snippet loadSnippet(const QString &id);
    bool saveSnippet(const Snippet &s);
    bool deleteSnippet(const QString &id);
    QList<Snippet> getAllSnippets() const;
    QString createSnippet(const QString &name, const QString &category);
    bool snippetExists(const QString &id) const;

    static int fuzzyMatchScore(const QString &query, const QString &target);
    QList<SearchResult> searchSnippets(const QString &query) const;
    QStringList getAllCategories() const;

signals:
    void snippetCreated(const QString &id);
    void snippetDeleted(const QString &id);
    void snippetModified(const QString &id);

private:
    QString basePath;
    QString presetPath;

    QString getSnippetPath(const QString &id) const;
    QJsonObject snippetToJson(const Snippet &s) const;
    Snippet jsonToSnippet(const QJsonObject &obj) const;
};
