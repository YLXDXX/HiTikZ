#include "snippet_manager.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QUuid>
#include <QJsonDocument>
#include <QSet>
#include <algorithm>

SnippetManager::SnippetManager(QObject *parent)
    : QObject(parent)
{
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    basePath = dataLocation + "/snippets/";
    presetPath = dataLocation + "/presets/";
    QDir().mkpath(basePath);
    QDir().mkpath(presetPath);
}

QString SnippetManager::getBasePath() const
{
    return basePath;
}

QString SnippetManager::getPresetPath() const
{
    return presetPath;
}

QString SnippetManager::getSnippetPath(const QString &id) const
{
    return basePath + id + "/";
}

bool SnippetManager::snippetExists(const QString &id) const
{
    return QDir(getSnippetPath(id)).exists();
}

QString SnippetManager::createSnippet(const QString &name, const QString &category)
{
    Snippet s;
    s.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    s.name = name;
    s.category = category;
    s.code = QStringLiteral("\\begin{tikzpicture}\n\n\\end{tikzpicture}");
    saveSnippet(s);
    emit snippetCreated(s.id);
    return s.id;
}

bool SnippetManager::saveSnippet(const Snippet &s)
{
    QString path = getSnippetPath(s.id);
    if (!QDir().mkpath(path))
        return false;

    QJsonObject json = snippetToJson(s);
    QJsonDocument doc(json);
    QFile metaFile(path + "meta.json");
    if (metaFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        metaFile.write(doc.toJson());
        metaFile.close();
    } else {
        return false;
    }

    QFile texFile(path + "snippet.tex");
    if (texFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        texFile.write(s.code.toUtf8());
        texFile.close();
        emit snippetModified(s.id);
        return true;
    }
    return false;
}

Snippet SnippetManager::loadSnippet(const QString &id)
{
    Snippet s;
    QString path = getSnippetPath(id);
    if (!QDir(path).exists())
        return s;

    QFile metaFile(path + "meta.json");
    if (metaFile.open(QIODevice::ReadOnly)) {
        QByteArray data = metaFile.readAll();
        metaFile.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            s = jsonToSnippet(doc.object());
        }
    }

    QFile texFile(path + "snippet.tex");
    if (texFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        s.code = QString::fromUtf8(texFile.readAll());
        texFile.close();
    }

    return s;
}

bool SnippetManager::deleteSnippet(const QString &id)
{
    QString path = getSnippetPath(id);
    QDir dir(path);
    if (!dir.exists())
        return false;
    dir.removeRecursively();
    emit snippetDeleted(id);
    return true;
}

QList<Snippet> SnippetManager::getAllSnippets() const
{
    QList<Snippet> snippets;
    QDir dir(basePath);
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        QString metaPath = basePath + entry + "/meta.json";
        if (!QFile::exists(metaPath))
            continue;

        QFile metaFile(metaPath);
        if (!metaFile.open(QIODevice::ReadOnly))
            continue;

        QByteArray data = metaFile.readAll();
        metaFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            Snippet s = jsonToSnippet(doc.object());
            if (!s.id.isEmpty()) {
                snippets.append(s);
            }
        }
    }
    return snippets;
}

QJsonObject SnippetManager::snippetToJson(const Snippet &s) const
{
    QJsonObject obj;
    obj["id"] = s.id;
    obj["name"] = s.name;
    obj["description"] = s.description;
    obj["category"] = s.category;
    obj["templateId"] = s.templateId;
    QJsonArray tagsArr;
    for (const QString &tag : s.tags)
        tagsArr.append(tag);
    obj["tags"] = tagsArr;
    return obj;
}

Snippet SnippetManager::jsonToSnippet(const QJsonObject &obj) const
{
    Snippet s;
    s.id = obj.value("id").toString();
    s.name = obj.value("name").toString();
    s.description = obj.value("description").toString();
    s.category = obj.value("category").toString();
    s.templateId = obj.value("templateId").toString();
    QJsonArray tagsArr = obj.value("tags").toArray();
    for (const QJsonValue &v : tagsArr)
        s.tags.append(v.toString());
    return s;
}

int SnippetManager::fuzzyMatchScore(const QString &query, const QString &target)
{
    if (query.isEmpty()) return 100;

    int score = 0;
    int qi = 0;
    int consecutive = 0;

    for (int ti = 0; ti < target.length() && qi < query.length(); ++ti) {
        if (query[qi].toLower() == target[ti].toLower()) {
            score += 10 + consecutive * 5;
            consecutive++;
            qi++;
        } else {
            consecutive = 0;
        }
    }

    return (qi == query.length()) ? score : 0;
}

QList<SearchResult> SnippetManager::searchSnippets(const QString &query) const
{
    QList<SearchResult> results;
    QList<Snippet> all = getAllSnippets();

    for (const Snippet &s : all) {
        int nameScore = fuzzyMatchScore(query, s.name);
        int descScore = fuzzyMatchScore(query, s.description) / 3;
        int totalScore = nameScore * 2 + descScore;

        if (totalScore > 0) {
            SearchResult r;
            r.snippet = s;
            r.score = totalScore;
            results.append(r);
        }
    }

    std::sort(results.begin(), results.end(),
        [](const SearchResult &a, const SearchResult &b) {
            return a.score > b.score;
        });

    return results;
}

QStringList SnippetManager::getAllCategories() const
{
    QSet<QString> cats;
    QList<Snippet> all = getAllSnippets();
    for (const Snippet &s : all) {
        if (!s.category.isEmpty()) {
            cats.insert(s.category);
        }
    }
    return cats.values();
}
