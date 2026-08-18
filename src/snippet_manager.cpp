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

bool SnippetManager::categoryMatches(const QString &snippetCategory,
                                     const QString &filterCategory)
{
    if (filterCategory.isEmpty())
        return true;
    return snippetCategory == filterCategory
        || snippetCategory.startsWith(filterCategory + QLatin1Char('/'));
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
    if (id.isEmpty())
        return false;

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

    double minOrder = 0.0;
    bool found = false;
    QList<Snippet> all = getAllSnippets();
    all.append(getAllPresets());
    for (const auto& snip : all) {
        if (snip.category == category) {
            if (!found || snip.sortOrder < minOrder) {
                minOrder = snip.sortOrder;
                found = true;
            }
        }
    }
    s.sortOrder = found ? minOrder - 1.0 : 0.0;

    saveSnippet(s);
    emit snippetCreated(s.id);
    return s.id;
}

bool SnippetManager::saveSnippet(const Snippet &s)
{
    if (s.id.isEmpty())
        return false;

    QString path;
    if (isPresetId(s.id))
        path = getPresetSnippetPath(s.id);
    else
        path = getSnippetPath(s.id);

    if (!QDir().mkpath(path))
        return false;

    QSaveFile texFile(path + "snippet.tex");
    if (!texFile.open(QIODevice::WriteOnly))
        return false;
    texFile.write(s.code.toUtf8());
    if (!texFile.commit())
        return false;

    QJsonObject json = snippetToJson(s);
    QJsonDocument doc(json);
    QSaveFile metaFile(path + "meta.json");
    if (!metaFile.open(QIODevice::WriteOnly))
        return false;
    metaFile.write(doc.toJson());
    if (!metaFile.commit())
        return false;

    removeSnippetFromSearchIndex(s.id);
    addSnippetToSearchIndex(s);
    invalidateCachesLight();
    emit snippetModified(s.id);
    return true;
}

Snippet SnippetManager::loadSnippet(const QString &id)
{
    if (id.isEmpty())
        return Snippet();

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
    if (id.isEmpty())
        return Snippet();

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
    if (id.isEmpty())
        return false;

    Snippet snip = loadSnippet(id);
    QString category = snip.category;

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

    if (!category.isEmpty()) {
        QList<Snippet> all = getAllSnippets();
        all.append(getAllPresets());
        bool hasMore = false;
        for (const Snippet &s : all) {
            if (s.category == category || s.category.startsWith(category + "/")) {
                hasMore = true;
                break;
            }
        }
        if (!hasMore) {
            addCategory(category);
        }
    }

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
    obj["compileCommand"] = s.compileCommand;
    obj["sortOrder"] = s.sortOrder;
    QJsonArray tagsArr;
    for (const QString &tag : s.tags)
        tagsArr.append(tag);
    obj["tags"] = tagsArr;
    QJsonArray imagesArr;
    for (const QString &img : s.images)
        imagesArr.append(img);
    obj["images"] = imagesArr;
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
    s.compileCommand = obj.value("compileCommand").toString();
    s.sortOrder = obj.value("sortOrder").toDouble();
    QJsonArray tagsArr = obj.value("tags").toArray();
    for (const QJsonValue &v : tagsArr)
        s.tags.append(v.toString());
    // Tolerate archives/meta.json written before the images field existed.
    QJsonArray imagesArr = obj.value("images").toArray();
    for (const QJsonValue &v : imagesArr)
        s.images.append(v.toString());
    return s;
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
                Snippet updated = loadSnippet(s.id);
                updated.category = newCategory;
                saveSnippet(updated);
                count++;
            } else if (s.category.startsWith(oldCategory + "/")) {
                Snippet updated = loadSnippet(s.id);
                updated.category = newCategory + "/" + s.category.mid(oldCategory.length() + 1);
                saveSnippet(updated);
                count++;
            }
        }
    };
    updateAll(getAllSnippets(false));
    updateAll(getAllPresets(false));
    if (count > 0)
        emit categoriesChanged();
    return count;
}

int SnippetManager::deleteCategory(const QString &category)
{
    if (category.isEmpty())
        return 0;

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

    removeCategoryFromPersisted(category);
    QStringList cats = loadAllPersistedCategories();
    for (int i = cats.size() - 1; i >= 0; --i) {
        if (cats[i] == category || cats[i].startsWith(category + "/"))
            cats.removeAt(i);
    }
    savePersistedCategories(cats);

    if (count > 0)
        emit categoriesChanged();
    return count;
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

void SnippetManager::reorderSnippets(const QStringList &orderedIds)
{
    if (orderedIds.size() <= 1) return;

    QSet<QString> orderedSet(orderedIds.begin(), orderedIds.end());

    QList<Snippet> all = getAllSnippets();
    all.append(getAllPresets());

    std::sort(all.begin(), all.end(), [](const Snippet &a, const Snippet &b) {
        return a.sortOrder < b.sortOrder;
    });

    int insertPos = -1;
    for (int i = 0; i < all.size(); ++i) {
        if (orderedSet.contains(all[i].id)) {
            insertPos = i;
            break;
        }
    }
    if (insertPos < 0) return;

    QStringList finalOrder;
    QSet<QString> placedSet;

    for (int i = 0; i < insertPos; ++i) {
        if (!orderedSet.contains(all[i].id)) {
            finalOrder.append(all[i].id);
            placedSet.insert(all[i].id);
        }
    }

    for (const QString &id : orderedIds) {
        finalOrder.append(id);
        placedSet.insert(id);
    }

    for (int i = insertPos; i < all.size(); ++i) {
        if (!placedSet.contains(all[i].id)) {
            finalOrder.append(all[i].id);
            placedSet.insert(all[i].id);
        }
    }

    for (int i = 0; i < finalOrder.size(); ++i) {
        Snippet s = loadSnippet(finalOrder[i]);
        if (s.id.isEmpty()) continue;
        if (s.sortOrder == static_cast<double>(i)) continue;
        s.sortOrder = static_cast<double>(i);
        saveSnippet(s);
    }
}

QString SnippetManager::categoryOrderFile()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + "/category_order.json";
}

QStringList SnippetManager::loadCategoryOrder() const
{
    QFile file(categoryOrderFile());
    if (!file.open(QIODevice::ReadOnly))
        return {};
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    QStringList order;
    if (doc.isArray()) {
        for (const QJsonValue &v : doc.array())
            order.append(v.toString());
    }
    return order;
}

void SnippetManager::saveCategoryOrder(const QStringList &order)
{
    QJsonArray arr;
    for (const QString &cat : order)
        arr.append(cat);
    QSaveFile file(categoryOrderFile());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
        file.commit();
    }
}

QString SnippetManager::categoryListFile() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + "/category_list.json";
}

QStringList SnippetManager::loadAllPersistedCategories() const
{
    QFile file(categoryListFile());
    if (!file.open(QIODevice::ReadOnly))
        return {};
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    QStringList cats;
    if (doc.isArray()) {
        for (const auto &v : doc.array())
            cats.append(v.toString());
    }
    return cats;
}

void SnippetManager::savePersistedCategories(const QStringList &cats)
{
    QJsonArray arr;
    for (const auto &cat : cats)
        arr.append(cat);
    QSaveFile file(categoryListFile());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
        file.commit();
    }
}

void SnippetManager::addCategory(const QString &category)
{
    if (category.isEmpty()) return;
    QStringList cats = loadAllPersistedCategories();
    if (!cats.contains(category)) {
        cats.append(category);
        savePersistedCategories(cats);
        invalidateCachesLight();
        emit categoriesChanged();
    }
}

void SnippetManager::removeCategoryFromPersisted(const QString &category)
{
    QStringList cats = loadAllPersistedCategories();
    if (cats.removeAll(category) > 0) {
        savePersistedCategories(cats);
    }
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

QStringList SnippetManager::supportedImageExtensions()
{
    return {QStringLiteral("png"),  QStringLiteral("jpg"), QStringLiteral("jpeg"),
            QStringLiteral("tif"),  QStringLiteral("tiff"), QStringLiteral("bmp"),
            QStringLiteral("gif"),  QStringLiteral("webp"), QStringLiteral("pdf")};
}

bool SnippetManager::isSupportedImageFile(const QString &fileName)
{
    const QString ext = QFileInfo(fileName).suffix().toLower();
    if (ext.isEmpty())
        return false;
    return supportedImageExtensions().contains(ext);
}

QString SnippetManager::imageStemForIndex(int index)
{
    if (index < 0)
        index = 0;
    QString stem;
    do {
        stem.prepend(QChar('a' + (index % 26)));
        index = index / 26 - 1;
    } while (index >= 0);
    return stem;
}

QString SnippetManager::getSnippetImageDir(const QString &id) const
{
    return isPresetId(id) ? getPresetSnippetPath(id) : getSnippetPath(id);
}

QStringList SnippetManager::getSnippetImagePaths(const QString &id)
{
    QStringList paths;
    if (id.isEmpty())
        return paths;

    const Snippet s = loadSnippet(id);
    if (s.id.isEmpty())
        return paths;

    const QString dir = getSnippetImageDir(id);
    for (const QString &name : s.images) {
        const QString path = dir + name;
        if (QFile::exists(path))
            paths.append(path);
    }
    return paths;
}

QString SnippetManager::addImageToSnippet(const QString &id, const QString &srcFilePath)
{
    Snippet s = loadSnippet(id);
    if (s.id.isEmpty())
        return QString();

    const QFileInfo srcInfo(srcFilePath);
    if (!srcInfo.exists() || !srcInfo.isFile())
        return QString();
    const QString ext = srcInfo.suffix().toLower();
    if (ext.isEmpty())
        return QString();

    const QString dir = getSnippetImageDir(id);

    // Collect stems that are already in use: those recorded in meta.json plus
    // any image files actually present in the directory (covers stale/unlisted
    // files so we never overwrite an existing image).
    QSet<QString> usedStems;
    for (const QString &name : s.images)
        usedStems.insert(QFileInfo(name).completeBaseName());
    const QStringList existingFiles = QDir(dir).entryList(QDir::Files);
    for (const QString &file : existingFiles) {
        if (isSupportedImageFile(file))
            usedStems.insert(QFileInfo(file).completeBaseName());
    }

    int index = 0;
    while (usedStems.contains(imageStemForIndex(index)))
        index++;
    const QString name = imageStemForIndex(index) + QStringLiteral(".") + ext;

    if (!QFile::copy(srcFilePath, dir + name))
        return QString();

    s.images.append(name);
    if (!saveSnippet(s)) {
        QFile::remove(dir + name);
        return QString();
    }
    return name;
}

QString SnippetManager::replaceSnippetImage(const QString &id, const QString &imageName,
                                            const QString &srcFilePath)
{
    Snippet s = loadSnippet(id);
    if (s.id.isEmpty() || imageName.isEmpty())
        return QString();

    const QFileInfo srcInfo(srcFilePath);
    if (!srcInfo.exists() || !srcInfo.isFile())
        return QString();
    const QString ext = srcInfo.suffix().toLower();
    if (ext.isEmpty())
        return QString();

    const QString dir = getSnippetImageDir(id);
    const QString stem = QFileInfo(imageName).completeBaseName();
    if (stem.isEmpty())
        return QString();
    const QString newName = stem + QStringLiteral(".") + ext;

    QFile::remove(dir + imageName);
    if (!QFile::copy(srcFilePath, dir + newName))
        return QString();

    int idx = s.images.indexOf(imageName);
    if (idx >= 0)
        s.images[idx] = newName;
    else
        s.images.append(newName);

    if (!saveSnippet(s)) {
        QFile::remove(dir + newName);
        return QString();
    }
    return newName;
}

bool SnippetManager::removeSnippetImage(const QString &id, const QString &imageName)
{
    if (id.isEmpty() || imageName.isEmpty())
        return false;

    Snippet s = loadSnippet(id);
    if (s.id.isEmpty())
        return false;

    const QString dir = getSnippetImageDir(id);
    QFile::remove(dir + imageName);

    const bool listed = s.images.removeAll(imageName) > 0;
    if (listed)
        saveSnippet(s);
    return true;
}

bool SnippetManager::copySnippetImageFiles(const QString &srcId, const QString &dstId)
{
    const Snippet src = loadSnippet(srcId);
    if (src.id.isEmpty())
        return false;

    const QString srcDir = getSnippetImageDir(srcId);
    const QString dstDir = getSnippetImageDir(dstId);

    bool ok = true;
    for (const QString &name : src.images) {
        const QString srcFile = srcDir + name;
        if (!QFile::exists(srcFile))
            continue;
        const QString dstFile = dstDir + name;
        if (QFile::exists(dstFile))
            QFile::remove(dstFile);
        if (!QFile::copy(srcFile, dstFile)) {
            qWarning() << "Failed to copy image file:" << srcFile << "→" << dstFile;
            ok = false;
        }
    }
    return ok;
}
