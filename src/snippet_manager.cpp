#include "snippet_manager.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QUuid>
#include <QJsonDocument>
#include <QSet>
#include <QProcess>
#include <QTemporaryDir>
#include <QDebug>
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

QString SnippetManager::getResourcePresetPath() const
{
    return QStringLiteral("resources/presets");
}

QString SnippetManager::getSnippetPath(const QString &id) const
{
    return basePath + id + "/";
}

QString SnippetManager::getPresetSnippetPath(const QString &id) const
{
    return presetPath + id + "/";
}

bool SnippetManager::snippetExists(const QString &id) const
{
    return QDir(getSnippetPath(id)).exists() || QDir(getPresetSnippetPath(id)).exists();
}

bool SnippetManager::isPresetId(const QString &id) const
{
    ensurePresetIdsCached();
    return presetIdsCache.contains(id);
}

void SnippetManager::ensurePresetIdsCached() const
{
    if (presetIdsCached) return;

    QDir dir(presetPath);
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        QString metaPath = presetPath + entry + "/meta.json";
        if (!QFile::exists(metaPath)) continue;
        QFile metaFile(metaPath);
        if (!metaFile.open(QIODevice::ReadOnly)) continue;
        QByteArray data = metaFile.readAll();
        metaFile.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QString id = doc.object().value("id").toString();
            if (!id.isEmpty()) presetIdsCache.insert(id);
        }
    }
    presetIdsCached = true;
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
    QString path;
    if (isPresetId(s.id))
        path = getPresetSnippetPath(s.id);
    else
        path = getSnippetPath(s.id);

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
    if (isPresetId(id))
        return loadPreset(id);

    QString path = getSnippetPath(id);
    if (!QDir(path).exists())
        return Snippet();

    Snippet s = loadMetaFromDir(path);
    loadCodeForSnippet(path, s);
    return s;
}

Snippet SnippetManager::loadPreset(const QString &id)
{
    QString path = getPresetSnippetPath(id);
    if (!QDir(path).exists())
        return Snippet();

    Snippet s = loadMetaFromDir(path);
    loadCodeForSnippet(path, s);
    s.isPreset = true;
    return s;
}

bool SnippetManager::deleteSnippet(const QString &id)
{
    QString path;
    if (isPresetId(id))
        path = getPresetSnippetPath(id);
    else
        path = getSnippetPath(id);

    QDir dir(path);
    if (!dir.exists())
        return false;
    dir.removeRecursively();

    if (presetIdsCached && presetIdsCache.contains(id)) {
        presetIdsCache.remove(id);
    }

    emit snippetDeleted(id);
    return true;
}

QList<Snippet> SnippetManager::getAllSnippets(bool loadCode) const
{
    QList<Snippet> snippets;
    QDir dir(basePath);
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        QString entryPath = basePath + entry;
        Snippet s = loadMetaFromDir(entryPath);
        if (s.id.isEmpty()) continue;
        if (loadCode) loadCodeForSnippet(entryPath, s);
        snippets.append(s);
    }
    return snippets;
}

QList<Snippet> SnippetManager::getAllPresets(bool loadCode) const
{
    QList<Snippet> presets;
    QDir dir(presetPath);
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        QString entryPath = presetPath + entry;
        Snippet s = loadMetaFromDir(entryPath);
        if (s.id.isEmpty()) continue;
        s.isPreset = true;
        if (loadCode) loadCodeForSnippet(entryPath, s);
        presets.append(s);
    }
    return presets;
}

QJsonObject SnippetManager::snippetToJson(const Snippet &s) const
{
    QJsonObject obj;
    obj["id"] = s.id;
    obj["name"] = s.name;
    obj["description"] = s.description;
    obj["category"] = s.category;
    obj["templateId"] = s.templateId;
    obj["packages"] = s.packages;
    obj["tikzLibraries"] = s.tikzLibraries;
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
    s.packages = obj.value("packages").toString();
    s.tikzLibraries = obj.value("tikzLibraries").toString();
    QJsonArray tagsArr = obj.value("tags").toArray();
    for (const QJsonValue &v : tagsArr)
        s.tags.append(v.toString());
    return s;
}

void SnippetManager::copyPresetsFromResources(const QString &resourceDir, const QString &destDir)
{
    QDir().mkpath(destDir);

    QDir resDir(resourceDir);
    if (!resDir.exists()) return;

    QStringList presetDirs = resDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &presetId : presetDirs) {
        QString srcDir = resourceDir + "/" + presetId;
        QString dstDir = destDir + "/" + presetId;
        if (QDir(dstDir).exists()) continue;

        QDir().mkpath(dstDir);

        QStringList files = QDir(srcDir).entryList(QDir::Files);
        for (const QString &file : files) {
            if (!QFile::copy(srcDir + "/" + file, dstDir + "/" + file)) {
                qWarning() << "Failed to copy preset file:" << srcDir + "/" + file;
            }
        }
    }
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

QList<SearchResult> SnippetManager::searchSnippets(const QString &query, bool includePresets) const
{
    QList<SearchResult> results;
    QList<Snippet> all = getAllSnippets();

    if (includePresets) {
        QList<Snippet> presets = getAllPresets();
        all.append(presets);
    }

    for (const Snippet &s : all) {
        int nameScore = fuzzyMatchScore(query, s.name);
        int descScore = fuzzyMatchScore(query, s.description) / 3;
        int totalScore = nameScore * 2 + descScore;

        if (totalScore > 0 || query.isEmpty()) {
            SearchResult r;
            r.snippet = s;
            r.score = totalScore > 0 ? totalScore : 0;
            results.append(r);
        }
    }

    std::sort(results.begin(), results.end(),
        [](const SearchResult &a, const SearchResult &b) {
            return a.score > b.score;
        });

    return results;
}

QStringList SnippetManager::getAllCategories(bool includePresets) const
{
    QSet<QString> cats;
    QList<Snippet> all = getAllSnippets();
    for (const Snippet &s : all) {
        if (!s.category.isEmpty()) {
            cats.insert(s.category);
        }
    }
    if (includePresets) {
        QList<Snippet> presets = getAllPresets();
        for (const Snippet &s : presets) {
            if (!s.category.isEmpty()) {
                cats.insert(s.category);
            }
        }
    }
    return cats.values();
}

QMap<QString, int> SnippetManager::getCategoryCounts(bool includePresets) const
{
    QMap<QString, int> counts;
    QList<Snippet> all = getAllSnippets();
    for (const Snippet &s : all) {
        if (!s.category.isEmpty()) {
            counts[s.category]++;
        }
    }
    if (includePresets) {
        QList<Snippet> presets = getAllPresets();
        for (const Snippet &s : presets) {
            if (!s.category.isEmpty()) {
                counts[s.category]++;
            }
        }
    }
    return counts;
}

bool SnippetManager::updateSnippetCategory(const QString &id, const QString &newCategory)
{
    Snippet s = loadSnippet(id);
    if (s.id.isEmpty()) return false;
    s.category = newCategory;
    return saveSnippet(s);
}

int SnippetManager::renameCategory(const QString &oldCategory, const QString &newCategory)
{
    int count = 0;
    auto updateAll = [&](const QList<Snippet> &list) {
        for (const Snippet &s : list) {
            if (s.category == oldCategory) {
                Snippet updated = s;
                updated.category = newCategory;
                saveSnippet(updated);
                count++;
            } else if (s.category.startsWith(oldCategory + "/")) {
                Snippet updated = s;
                updated.category = newCategory + "/" + s.category.mid(oldCategory.length() + 1);
                saveSnippet(updated);
                count++;
            }
        }
    };
    updateAll(getAllSnippets(true));
    updateAll(getAllPresets(true));
    if (count > 0)
        emit categoriesChanged();
    return count;
}

int SnippetManager::deleteCategory(const QString &category)
{
    int count = 0;
    QList<Snippet> all = getAllSnippets();
    for (const Snippet &s : all) {
        if (s.category == category || s.category.startsWith(category + "/")) {
            deleteSnippet(s.id);
            count++;
        }
    }
    QList<Snippet> presets = getAllPresets();
    for (const Snippet &s : presets) {
        if (s.category == category || s.category.startsWith(category + "/")) {
            deleteSnippet(s.id);
            count++;
        }
    }
    if (count > 0)
        emit categoriesChanged();
    return count;
}

bool SnippetManager::exportSnippetZip(const QString &id, const QString &zipPath)
{
    QString srcDir;
    if (isPresetId(id))
        srcDir = getPresetSnippetPath(id);
    else
        srcDir = getSnippetPath(id);

    while (srcDir.endsWith('/'))
        srcDir.chop(1);

    QDir dir(srcDir);
    if (!dir.exists()) return false;

    if (QFile::exists(zipPath))
        QFile::remove(zipPath);

    QProcess tar;
    QString parentDir = QFileInfo(srcDir).absolutePath();
    QString dirName = QFileInfo(srcDir).fileName();
    QStringList args;
    args << "-czf" << zipPath << "-C" << parentDir << dirName;
    tar.start("tar", args);
    tar.waitForFinished(10000);
    return tar.exitCode() == 0 && QFile::exists(zipPath);
}

bool SnippetManager::exportSnippetsZip(const QStringList &ids, const QString &zipPath)
{
    if (ids.isEmpty()) return false;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) return false;

    for (const QString &id : ids) {
        QString srcDir;
        if (isPresetId(id))
            srcDir = getPresetSnippetPath(id);
        else
            srcDir = getSnippetPath(id);

        while (srcDir.endsWith('/'))
            srcDir.chop(1);

        QDir dir(srcDir);
        if (!dir.exists()) continue;

        QString dstDir = tempDir.path() + "/" + id;
        QDir().mkpath(dstDir);

        QStringList files = dir.entryList(QDir::Files);
        for (const QString &file : files) {
            if (!QFile::copy(srcDir + "/" + file, dstDir + "/" + file)) {
                qWarning() << "Failed to copy file for export:" << srcDir + "/" + file;
                return false;
            }
        }
    }

    if (QFile::exists(zipPath))
        QFile::remove(zipPath);

    QProcess tar;
    QStringList args;
    args << "-czf" << zipPath << "-C" << tempDir.path() << ".";
    tar.start("tar", args);
    tar.waitForFinished(10000);
    return tar.exitCode() == 0 && QFile::exists(zipPath);
}

bool SnippetManager::batchUpdateCategory(const QStringList &ids, const QString &newCategory)
{
    if (ids.isEmpty()) return false;
    int count = 0;
    for (const QString &id : ids) {
        Snippet s = loadSnippet(id);
        if (s.id.isEmpty()) continue;
        s.category = newCategory;
        if (saveSnippet(s)) count++;
    }
    if (count > 0)
        emit categoriesChanged();
    return count > 0;
}

int SnippetManager::batchDeleteSnippets(const QStringList &ids)
{
    int count = 0;
    for (const QString &id : ids) {
        if (deleteSnippet(id)) count++;
    }
    if (count > 0)
        emit categoriesChanged();
    return count;
}

QStringList SnippetManager::importSnippetsZip(const QString &zipPath)
{
    QStringList importedIds;

    if (!QFile::exists(zipPath)) return importedIds;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) return importedIds;

    QProcess unzip;
    unzip.start("tar", QStringList() << "-xzf" << zipPath << "-C" << tempDir.path());
    unzip.waitForFinished(10000);

    if (unzip.exitCode() != 0) {
        QProcess unzipFallback;
        unzipFallback.start("unzip", QStringList() << "-o" << zipPath << "-d" << tempDir.path());
        unzipFallback.waitForFinished(10000);
        if (unzipFallback.exitCode() != 0)
            return importedIds;
    }

    QDir extractDir(tempDir.path());
    QStringList subDirs = extractDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &subDir : subDirs) {
        QString metaJsonPath = tempDir.path() + "/" + subDir + "/meta.json";
        if (!QFile::exists(metaJsonPath)) {
            QString snippetTexPath = tempDir.path() + "/" + subDir + "/snippet.tex";
            if (!QFile::exists(snippetTexPath)) continue;
        }

        QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString destDir = basePath + newId + "/";
        QDir().mkpath(destDir);

        QStringList files = QDir(tempDir.path() + "/" + subDir).entryList(QDir::Files);
        bool importOk = true;
        for (const QString &file : files) {
            if (!QFile::copy(tempDir.path() + "/" + subDir + "/" + file, destDir + file)) {
                qWarning() << "Failed to import file:" << tempDir.path() + "/" + subDir + "/" + file;
                importOk = false;
                break;
            }
        }
        if (!importOk) continue;

        if (QFile::exists(destDir + "meta.json")) {
            QFile metaFile(destDir + "meta.json");
            if (metaFile.open(QIODevice::ReadOnly)) {
                QByteArray data = metaFile.readAll();
                metaFile.close();
                QJsonDocument doc = QJsonDocument::fromJson(data);
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    obj["id"] = newId;
                    QJsonDocument newDoc(obj);
                    if (metaFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                        metaFile.write(newDoc.toJson());
                        metaFile.close();
                    }
                }
            }
        }

        importedIds.append(newId);
        emit snippetCreated(newId);
    }

    if (!importedIds.isEmpty())
        emit categoriesChanged();

    return importedIds;
}

Snippet SnippetManager::loadMetaFromDir(const QString &dirPath) const
{
    Snippet s;
    QString metaPath = dirPath + "/meta.json";
    if (!QFile::exists(metaPath))
        return s;

    QFile metaFile(metaPath);
    if (!metaFile.open(QIODevice::ReadOnly))
        return s;

    QByteArray data = metaFile.readAll();
    metaFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
        s = jsonToSnippet(doc.object());
    }
    return s;
}

void SnippetManager::loadCodeForSnippet(const QString &dirPath, Snippet &s) const
{
    QString texPath = dirPath + "/snippet.tex";
    QFile texFile(texPath);
    if (texFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        s.code = QString::fromUtf8(texFile.readAll());
        texFile.close();
    }
}
