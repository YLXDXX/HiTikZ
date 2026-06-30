#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>
#include <cassert>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    int failed = 0;

    QString draftDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/draft_test/";
    QDir().mkpath(draftDir);

    // Test 1: Write and verify draft JSON format (full metadata)
    {
        QJsonObject obj;
        obj["snippetId"] = "test-id-001";
        obj["code"] = "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}";
        obj["name"] = "Test Draft Name";
        obj["description"] = "A test description";
        obj["tags"] = "math, geometry, test";
        obj["packages"] = "tikz-3dplot, amsmath";
        obj["tikzLibraries"] = "calc, arrows, shapes";
        obj["templateId"] = "default_math";

        QString draftPath = draftDir + "test-id-001.json";
        QFile file(draftPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(QJsonDocument(obj).toJson());
            file.close();
        }

        QFile readFile(draftPath);
        if (readFile.open(QIODevice::ReadOnly)) {
            QByteArray data = readFile.readAll();
            readFile.close();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isObject()) {
                qDebug() << "FAIL: Test 1a - JSON is not object";
                failed++;
            } else {
                QJsonObject readObj = doc.object();
                if (readObj.value("snippetId").toString() != "test-id-001") {
                    qDebug() << "FAIL: Test 1b - snippetId mismatch";
                    failed++;
                } else if (readObj.value("name").toString() != "Test Draft Name") {
                    qDebug() << "FAIL: Test 1c - name mismatch";
                    failed++;
                } else if (readObj.value("description").toString() != "A test description") {
                    qDebug() << "FAIL: Test 1d - description mismatch";
                    failed++;
                } else if (readObj.value("tags").toString() != "math, geometry, test") {
                    qDebug() << "FAIL: Test 1e - tags mismatch";
                    failed++;
                } else if (readObj.value("packages").toString() != "tikz-3dplot, amsmath") {
                    qDebug() << "FAIL: Test 1f - packages mismatch";
                    failed++;
                } else if (readObj.value("tikzLibraries").toString() != "calc, arrows, shapes") {
                    qDebug() << "FAIL: Test 1g - tikzLibraries mismatch";
                    failed++;
                } else if (readObj.value("templateId").toString() != "default_math") {
                    qDebug() << "FAIL: Test 1h - templateId mismatch";
                    failed++;
                } else {
                    qDebug() << "PASS: Test 1 - Full metadata roundtrip";
                }
            }
        }
        QFile::remove(draftPath);
    }

    // Test 2: Scratch draft (no snippetId) format
    {
        QJsonObject obj;
        obj["snippetId"] = QString();
        obj["code"] = "\\begin{tikzpicture}\\draw(0,0)--(2,2);\\end{tikzpicture}";
        obj["name"] = "未命名草稿";
        obj["description"] = QString();
        obj["tags"] = QString();
        obj["packages"] = QString();
        obj["tikzLibraries"] = QString();
        obj["templateId"] = QString();

        QString draftPath = draftDir + "scratch.json";
        QFile file(draftPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(QJsonDocument(obj).toJson());
            file.close();
        }

        QFile readFile(draftPath);
        if (readFile.open(QIODevice::ReadOnly)) {
            QByteArray data = readFile.readAll();
            readFile.close();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject readObj = doc.object();

            if (!doc.isObject()) {
                qDebug() << "FAIL: Test 2a - JSON is not object";
                failed++;
            } else if (!readObj.value("snippetId").toString().isEmpty()) {
                qDebug() << "FAIL: Test 2b - snippetId should be empty";
                failed++;
            } else if (readObj.value("code").toString().isEmpty()) {
                qDebug() << "FAIL: Test 2c - code should not be empty";
                failed++;
            } else {
                qDebug() << "PASS: Test 2 - Scratch draft format";
            }
        }
        QFile::remove(draftPath);
    }

    // Test 3: Draft directory scan
    {
        int fileCount = 3;
        for (int i = 0; i < fileCount; ++i) {
            QJsonObject obj;
            obj["snippetId"] = QString("scan-id-%1").arg(i);
            obj["code"] = "\\begin{tikzpicture}\\end{tikzpicture}";
            obj["name"] = QString("Draft %1").arg(i);
            obj["description"] = QString("Desc %1").arg(i);
            obj["tags"] = "test";
            obj["packages"] = QString();
            obj["tikzLibraries"] = QString();
            obj["templateId"] = QString();

            QFile file(draftDir + QString("scan-id-%1.json").arg(i));
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                file.write(QJsonDocument(obj).toJson());
                file.close();
            }
        }

        QDir dir(draftDir);
        QStringList found = dir.entryList(QStringList() << "*.json", QDir::Files);
        int actualCount = 0;
        for (const QString &fname : found) {
            QString fpath = draftDir + fname;
            QFile f(fpath);
            if (f.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
                f.close();
                if (doc.isObject() && !doc.object().value("code").toString().trimmed().isEmpty())
                    actualCount++;
            }
        }

        if (actualCount != fileCount) {
            qDebug() << "FAIL: Test 3 - Expected" << fileCount << "drafts, found" << actualCount;
            failed++;
        } else {
            qDebug() << "PASS: Test 3 - Draft directory scan";
        }

        for (int i = 0; i < fileCount; ++i)
            QFile::remove(draftDir + QString("scan-id-%1.json").arg(i));
    }

    // Test 4: Empty code drafts should be filtered
    {
        QJsonObject obj;
        obj["snippetId"] = "empty-code";
        obj["code"] = "   ";
        obj["name"] = "Empty Draft";
        obj["description"] = QString();
        obj["tags"] = QString();
        obj["packages"] = QString();
        obj["tikzLibraries"] = QString();
        obj["templateId"] = QString();

        QString draftPath = draftDir + "empty-code.json";
        QFile file(draftPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(QJsonDocument(obj).toJson());
            file.close();
        }

        QFile readFile(draftPath);
        if (readFile.open(QIODevice::ReadOnly)) {
            QByteArray data = readFile.readAll();
            readFile.close();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject readObj = doc.object();
            QString code = readObj.value("code").toString();
            bool shouldSkip = code.trimmed().isEmpty();
            if (!shouldSkip) {
                qDebug() << "FAIL: Test 4 - Empty code not detected";
                failed++;
            } else {
                qDebug() << "PASS: Test 4 - Empty code draft correctly filtered";
            }
        }
        QFile::remove(draftPath);
    }

    // Cleanup
    QDir(draftDir).removeRecursively();

    if (failed > 0) {
        qDebug() << "\n" << failed << "test(s) failed!";
        return 1;
    }

    qDebug() << "\nAll draft recovery tests passed!";
    return 0;
}
