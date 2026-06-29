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
#include <QEventLoop>
#include <QTimer>
#include <QSaveFile>
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
    QSaveFile metaFile(path + "meta.json");
    if (metaFile.open(QIODevice::WriteOnly)) {
        metaFile.write(doc.toJson());
        if (!metaFile.commit())
            return false;
    } else {
        return false;
    }

    QSaveFile texFile(path + "snippet.tex");
    if (texFile.open(QIODevice::WriteOnly)) {
        texFile.write(s.code.toUtf8());
        if (!texFile.commit()) {
            return false;
        }
        removeSnippetFromSearchIndex(s.id);
        addSnippetToSearchIndex(s);
        invalidateCachesLight();
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

    removeSnippetFromSearchIndex(id);
    invalidateCachesLight();
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

    QString normQuery = query.normalized(QString::NormalizationForm_C).toCaseFolded();
    QString normTarget = target.normalized(QString::NormalizationForm_C).toCaseFolded();

    int score = 0;
    int qi = 0;
    int consecutive = 0;

    for (int ti = 0; ti < normTarget.length() && qi < normQuery.length(); ++ti) {
        if (normQuery[qi] == normTarget[ti]) {
            score += 10 + consecutive * 5;
            consecutive++;
            qi++;
        } else {
            consecutive = 0;
        }
    }

    return (qi == normQuery.length()) ? score : 0;
}

void SnippetManager::ensureSearchIndexBuilt() const
{
    if (m_searchIndex.built) return;

    m_searchIndex.bigramIndex.clear();
    m_searchIndex.allTexts.clear();
    m_searchIndex.allIds.clear();

    QList<Snippet> all = getAllSnippets();
    QList<Snippet> presets = getAllPresets();
    all.append(presets);

    for (int idx = 0; idx < all.size(); ++idx) {
        const Snippet &s = all.at(idx);
        m_searchIndex.allIds.append(s.id);

        QString text = (s.name + " " + s.description)
            .normalized(QString::NormalizationForm_C)
            .toCaseFolded();
        m_searchIndex.allTexts.append(text);

        for (int i = 0; i < text.length() - 1; ++i) {
            QString bigram = text.mid(i, 2);
            m_searchIndex.bigramIndex[bigram].insert(idx);
        }
    }

    m_searchIndex.built = true;
}

void SnippetManager::addSnippetToSearchIndex(const Snippet &s) const
{
    if (!m_searchIndex.built) return;

    QString text = (s.name + " " + s.description)
        .normalized(QString::NormalizationForm_C)
        .toCaseFolded();

    int idx = m_searchIndex.allTexts.size();
    m_searchIndex.allIds.append(s.id);
    m_searchIndex.allTexts.append(text);

    for (int i = 0; i < text.length() - 1; ++i) {
        QString bigram = text.mid(i, 2);
        m_searchIndex.bigramIndex[bigram].insert(idx);
    }
}

void SnippetManager::removeSnippetFromSearchIndex(const QString &id) const
{
    if (!m_searchIndex.built) return;

    int targetIdx = -1;
    for (int i = 0; i < m_searchIndex.allIds.size(); ++i) {
        if (m_searchIndex.allIds[i] == id) {
            targetIdx = i;
            break;
        }
    }
    if (targetIdx < 0) return;

    const QString &text = m_searchIndex.allTexts[targetIdx];
    for (int i = 0; i < text.length() - 1; ++i) {
        QString bigram = text.mid(i, 2);
        m_searchIndex.bigramIndex[bigram].remove(targetIdx);
    }

    int lastIdx = m_searchIndex.allTexts.size() - 1;
    if (targetIdx != lastIdx) {
        const QString &lastText = m_searchIndex.allTexts[lastIdx];
        const QString &lastId = m_searchIndex.allIds[lastIdx];
        for (int i = 0; i < lastText.length() - 1; ++i) {
            QString bigram = lastText.mid(i, 2);
            m_searchIndex.bigramIndex[bigram].remove(lastIdx);
            m_searchIndex.bigramIndex[bigram].insert(targetIdx);
        }
        m_searchIndex.allTexts[targetIdx] = lastText;
        m_searchIndex.allIds[targetIdx] = lastId;
    }
    m_searchIndex.allTexts.removeLast();
    m_searchIndex.allIds.removeLast();
}

QList<SearchResult> SnippetManager::searchSnippets(const QString &query, bool includePresets) const
{
    QList<SearchResult> results;

    if (query.isEmpty() || query.length() < 2) {
        QList<Snippet> all = getAllSnippets();
        if (includePresets) {
            QList<Snippet> presets = getAllPresets();
            all.append(presets);
        }
        for (const Snippet &s : all) {
            if (query.isEmpty()) {
                SearchResult r;
                r.snippet = s;
                r.score = 0;
                results.append(r);
            } else {
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
        }
        std::sort(results.begin(), results.end(),
            [](const SearchResult &a, const SearchResult &b) {
                return a.score > b.score;
            });
        return results;
    }

    ensureSearchIndexBuilt();

    QString normQuery = query.normalized(QString::NormalizationForm_C).toCaseFolded();
    QSet<int> candidates;

    bool firstBigram = true;
    for (int i = 0; i < normQuery.length() - 1; ++i) {
        QString bigram = normQuery.mid(i, 2);
        if (m_searchIndex.bigramIndex.contains(bigram)) {
            if (firstBigram) {
                candidates = m_searchIndex.bigramIndex[bigram];
                firstBigram = false;
            } else {
                candidates.intersect(m_searchIndex.bigramIndex[bigram]);
            }
        }
    }

    if (candidates.isEmpty() && firstBigram) return results;

    QList<Snippet> all = getAllSnippets();
    if (includePresets) {
        QList<Snippet> presets = getAllPresets();
        all.append(presets);
    }

    QHash<QString, Snippet> idToSnippet;
    for (const Snippet &s : all)
        idToSnippet[s.id] = s;

    for (int idx : candidates) {
        if (idx >= m_searchIndex.allIds.size()) continue;
        QString sid = m_searchIndex.allIds[idx];
        if (!idToSnippet.contains(sid)) continue;
        Snippet s = idToSnippet[sid];

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

QStringList SnippetManager::getAllCategories(bool includePresets) const
{
    if (!includePresets) {
        QSet<QString> cats;
        QList<Snippet> all = getAllSnippets();
        for (const Snippet &s : all) {
            if (!s.category.isEmpty()) cats.insert(s.category);
        }
        return cats.values();
    }

    ensureCountsCached();
    return m_cachedCategoryCounts.keys();
}

QMap<QString, int> SnippetManager::getCategoryCounts(bool includePresets) const
{
    if (!includePresets) {
        QMap<QString, int> counts;
        QList<Snippet> all = getAllSnippets();
        for (const Snippet &s : all) {
            if (!s.category.isEmpty()) counts[s.category]++;
        }
        return counts;
    }

    ensureCountsCached();
    return m_cachedCategoryCounts;
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

static bool runProcessSync(QProcess &proc, int timeoutMs = 30000)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(&proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    if (proc.state() == QProcess::NotRunning)
        return false;
    loop.exec();
    return proc.state() == QProcess::NotRunning;
}

int SnippetManager::getUncategorizedCount(bool includePresets) const
{
    int count = 0;
    QList<Snippet> all = getAllSnippets();
    for (const Snippet &s : all) {
        if (s.category.isEmpty()) count++;
    }
    if (includePresets) {
        QList<Snippet> presets = getAllPresets();
        for (const Snippet &s : presets) {
            if (s.category.isEmpty()) count++;
        }
    }
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
    runProcessSync(tar, 10000);
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
    runProcessSync(tar, 10000);
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
    runProcessSync(unzip, 10000);

    if (unzip.exitCode() != 0) {
        QProcess unzipFallback;
        unzipFallback.start("unzip", QStringList() << "-o" << zipPath << "-d" << tempDir.path());
        runProcessSync(unzipFallback, 10000);
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
                    obj.remove("isPreset");
                    QJsonDocument newDoc(obj);
                    QSaveFile saveFile(destDir + "meta.json");
                    if (saveFile.open(QIODevice::WriteOnly)) {
                        saveFile.write(newDoc.toJson());
                        saveFile.commit();
                    }
                }
            }
        } else {
            Snippet newSnip;
            newSnip.id = newId;
            newSnip.name = subDir;
            saveSnippet(newSnip);
        }

        importedIds.append(newId);
        emit snippetCreated(newId);
    }

    if (!importedIds.isEmpty()) {
        invalidateCaches();
        emit categoriesChanged();
    }

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

void SnippetManager::invalidateCaches()
{
    m_countsCached = false;
    m_cachedCategoryCounts.clear();
    presetIdsCached = false;
    presetIdsCache.clear();
    m_searchIndex.built = false;
    m_searchIndex.bigramIndex.clear();
    m_searchIndex.allTexts.clear();
    m_searchIndex.allIds.clear();
}

void SnippetManager::invalidateCachesLight() const
{
    m_countsCached = false;
    m_cachedCategoryCounts.clear();
    presetIdsCached = false;
    presetIdsCache.clear();
}

void SnippetManager::ensureCountsCached() const
{
    if (m_countsCached) return;

    m_cachedCategoryCounts.clear();
    QList<Snippet> all = getAllSnippets();
    for (const Snippet &s : all) {
        if (!s.category.isEmpty())
            m_cachedCategoryCounts[s.category]++;
    }
    QList<Snippet> presets = getAllPresets();
    for (const Snippet &s : presets) {
        if (!s.category.isEmpty())
            m_cachedCategoryCounts[s.category]++;
    }
    m_countsCached = true;
}
