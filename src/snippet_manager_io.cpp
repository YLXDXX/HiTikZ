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

static bool runProcessSync(QProcess &proc, int timeoutMs = 30000)
{
    if (!proc.waitForStarted(3000))
        return false;
    if (!proc.waitForFinished(timeoutMs))
        return false;
    return true;
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

QStringList SnippetManager::importSnippetsZip(const QString &zipPath)
{
    QStringList importedIds;

    if (!QFile::exists(zipPath)) return importedIds;

    QTemporaryDir tempDir;
    if (!tempDir.isValid()) return importedIds;

    QString extractSubDir = tempDir.path() + "/extracted/";
    QDir().mkpath(extractSubDir);

    QProcess unzip;
    unzip.start("tar", QStringList() << "-xzf" << zipPath << "-C" << extractSubDir);
    runProcessSync(unzip, 10000);

    if (unzip.exitCode() != 0) {
        QDir(extractSubDir).removeRecursively();
        QDir().mkpath(extractSubDir);
        QProcess unzipFallback;
        unzipFallback.start("unzip", QStringList() << "-o" << zipPath << "-d" << extractSubDir);
        runProcessSync(unzipFallback, 10000);
        if (unzipFallback.exitCode() != 0)
            return importedIds;
    }

    QDir extractDir(extractSubDir);
    QStringList subDirs = extractDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &subDir : subDirs) {
        QString metaJsonPath = extractSubDir + "/" + subDir + "/meta.json";
        if (!QFile::exists(metaJsonPath)) {
            QString snippetTexPath = extractSubDir + "/" + subDir + "/snippet.tex";
            if (!QFile::exists(snippetTexPath)) continue;
        }

        QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString destDir = basePath + newId + "/";
        QDir().mkpath(destDir);

        QStringList files = QDir(extractSubDir + "/" + subDir).entryList(QDir::Files);
        bool importOk = true;
        for (const QString &file : files) {
            if (!QFile::copy(extractSubDir + "/" + subDir + "/" + file, destDir + file)) {
                qWarning() << "Failed to import file:" << extractSubDir + "/" + subDir + "/" + file;
                importOk = false;
                break;
            }
        }
        if (!importOk) {
            QDir(destDir).removeRecursively();
            continue;
        }

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
            QFile texFile(destDir + "snippet.tex");
            if (texFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                newSnip.code = QString::fromUtf8(texFile.readAll());
                texFile.close();
            }
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
