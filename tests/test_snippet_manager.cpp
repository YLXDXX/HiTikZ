#include "snippet_manager.h"
#include <QCoreApplication>
#include <QDebug>
#include <cassert>
#include <QDir>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    SnippetManager mgr;

    // Test 1: Base path exists
    {
        QString basePath = mgr.getBasePath();
        assert(!basePath.isEmpty());
        assert(QDir(basePath).exists());
        qDebug() << "PASS: Test 1 - Base path exists:" << basePath;
    }

    // Test 2: Create snippet
    {
        QString id = mgr.createSnippet("Test Snippet", "math/geometry");
        assert(!id.isEmpty());
        assert(mgr.snippetExists(id));
        qDebug() << "PASS: Test 2 - Create snippet, id:" << id;

        // Test 3: Load snippet
        Snippet s = mgr.loadSnippet(id);
        assert(s.id == id);
        assert(s.name == "Test Snippet");
        assert(s.category == "math/geometry");
        assert(s.code.contains("tikzpicture"));
        qDebug() << "PASS: Test 3 - Load snippet matches";

        // Test 4: Update snippet
        s.description = "A test description";
        s.tags = QStringList{"test", "geometry"};
        s.code = "\\begin{tikzpicture}\\draw (0,0)--(1,1);\\end{tikzpicture}";
        assert(mgr.saveSnippet(s));
        Snippet s2 = mgr.loadSnippet(id);
        assert(s2.description == "A test description");
        assert(s2.tags.size() == 2);
        assert(s2.tags.contains("test"));
        assert(s2.code == s.code);
        qDebug() << "PASS: Test 4 - Update snippet";

        // Test 5: GetAllSnippets
        QList<Snippet> all = mgr.getAllSnippets();
        assert(all.size() >= 1);
        bool found = false;
        for (const auto &sn : all) {
            if (sn.id == id) {
                found = true;
                break;
            }
        }
        assert(found);
        qDebug() << "PASS: Test 5 - getAllSnippets contains new snippet";

        // Test 6: Delete snippet
        assert(mgr.deleteSnippet(id));
        assert(!mgr.snippetExists(id));
        qDebug() << "PASS: Test 6 - Delete snippet";

        // Test 7: Loading non-existent snippet returns empty
        Snippet empty = mgr.loadSnippet(id);
        assert(empty.id.isEmpty());
        qDebug() << "PASS: Test 7 - Load non-existent returns empty";
    }

    // Test 8: getAllSnippets with loadCode
    {
        QString id = mgr.createSnippet("LoadCode Test", "test/loadcode");
        Snippet s = mgr.loadSnippet(id);
        s.code = "\\begin{tikzpicture}\\draw (0,0)--(2,2);\\end{tikzpicture}";
        mgr.saveSnippet(s);

        QList<Snippet> noCode = mgr.getAllSnippets(false);
        bool foundNoCode = false;
        for (const auto &sn : noCode) {
            if (sn.id == id) {
                assert(sn.code.isEmpty());
                foundNoCode = true;
                break;
            }
        }
        assert(foundNoCode);
        qDebug() << "PASS: Test 8a - getAllSnippets(false) does not load code";

        QList<Snippet> withCode = mgr.getAllSnippets(true);
        bool foundWithCode = false;
        for (const auto &sn : withCode) {
            if (sn.id == id) {
                assert(sn.code == s.code);
                foundWithCode = true;
                break;
            }
        }
        assert(foundWithCode);
        qDebug() << "PASS: Test 8b - getAllSnippets(true) loads code correctly";

        mgr.deleteSnippet(id);
    }

    // Test 9: renameCategory preserves snippet code
    {
        QString id = mgr.createSnippet("RenameCat Test", "test/renamecat");
        Snippet s = mgr.loadSnippet(id);
        s.code = "\\begin{tikzpicture}\\draw (0,0)--(3,3);\\end{tikzpicture}";
        mgr.saveSnippet(s);

        mgr.renameCategory("test/renamecat", "test/renamed");
        Snippet reloaded = mgr.loadSnippet(id);
        assert(reloaded.code == s.code);
        assert(reloaded.category == "test/renamed");
        qDebug() << "PASS: Test 9 - renameCategory preserves snippet code";

        mgr.deleteSnippet(id);
    }

    // Test 10: ZIP export and import
    {
        QString id = mgr.createSnippet("ZipTest", "test/zip");
        Snippet s = mgr.loadSnippet(id);
        s.description = "ZIP export/import test";
        s.code = "\\begin{tikzpicture}\\draw (0,0)--(4,4);\\end{tikzpicture}";
        s.tags = QStringList{"zip", "test"};
        mgr.saveSnippet(s);

        QString zipPath = QDir::tempPath() + "/test_export.tar.gz";
        if (QFile::exists(zipPath)) QFile::remove(zipPath);
        assert(mgr.exportSnippetZip(id, zipPath));
        assert(QFile::exists(zipPath));
        qDebug() << "PASS: Test 10a - Export snippet to archive";

        QStringList importedIds = mgr.importSnippetsZip(zipPath);
        assert(!importedIds.isEmpty());
        qDebug() << "PASS: Test 10b - Import snippet from archive";

        QString newId = importedIds.first();
        assert(mgr.snippetExists(newId));
        assert(!mgr.isPresetId(newId));
        Snippet imported = mgr.loadSnippet(newId);
        assert(imported.name == s.name);
        assert(imported.description == s.description);
        assert(imported.code == s.code);
        assert(imported.tags == s.tags);
        qDebug() << "PASS: Test 10c - Imported data matches original";

        QFile::remove(zipPath);
        mgr.deleteSnippet(id);
        mgr.deleteSnippet(newId);
    }

    qDebug() << "\nAll tests passed!";
    return 0;
}
