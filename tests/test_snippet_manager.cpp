#include "snippet_manager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

static int g_failed = 0;

#define CHECK(expr, msg) \
    do { \
        if (!(expr)) { \
            qDebug() << "FAIL:" << msg; \
            g_failed++; \
        } \
    } while (0)

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    SnippetManager mgr;

    // Test 1: Base path exists
    {
        QString basePath = mgr.getBasePath();
        CHECK(!basePath.isEmpty(), "Base path should not be empty");
        CHECK(QDir(basePath).exists(), "Base path should exist");
        qDebug() << "PASS: Test 1 - Base path exists:" << basePath;
    }

    // Test 2: Create snippet
    {
        QString id = mgr.createSnippet("Test Snippet", "math/geometry");
        CHECK(!id.isEmpty(), "Should create snippet");
        CHECK(mgr.snippetExists(id), "Snippet should exist after creation");
        qDebug() << "PASS: Test 2 - Create snippet, id:" << id;

        // Test 3: Load snippet
        Snippet s = mgr.loadSnippet(id);
        CHECK(s.id == id, "Loaded snippet id should match");
        CHECK(s.name == "Test Snippet", "Loaded snippet name should match");
        CHECK(s.category == "math/geometry", "Loaded snippet category should match");
        CHECK(s.code.contains("tikzpicture"), "Loaded snippet should contain tikzpicture");
        qDebug() << "PASS: Test 3 - Load snippet matches";

        // Test 4: Update snippet
        s.description = "A test description";
        s.tags = QStringList{"test", "geometry"};
        s.code = "\\begin{tikzpicture}\\draw (0,0)--(1,1);\\end{tikzpicture}";
        bool saved = mgr.saveSnippet(s);
        CHECK(saved, "Save snippet should succeed");
        Snippet s2 = mgr.loadSnippet(id);
        CHECK(s2.description == "A test description", "Description should be updated");
        CHECK(s2.tags.size() == 2, "Should have 2 tags");
        CHECK(s2.tags.contains("test"), "Should contain 'test' tag");
        CHECK(s2.code == s.code, "Code should match after update");
        qDebug() << "PASS: Test 4 - Update snippet";

        // Test 5: GetAllSnippets
        QList<Snippet> all = mgr.getAllSnippets();
        CHECK(all.size() >= 1, "Should have at least 1 snippet");
        bool found = false;
        for (const auto &sn : all) {
            if (sn.id == id) { found = true; break; }
        }
        CHECK(found, "Created snippet should be in getAllSnippets");
        qDebug() << "PASS: Test 5 - getAllSnippets contains new snippet";

        // Test 6: Delete snippet
        bool deleted = mgr.deleteSnippet(id);
        CHECK(deleted, "Delete snippet should succeed");
        CHECK(!mgr.snippetExists(id), "Snippet should not exist after deletion");
        qDebug() << "PASS: Test 6 - Delete snippet";

        // Test 7: Loading non-existent snippet returns empty
        Snippet empty = mgr.loadSnippet(id);
        CHECK(empty.id.isEmpty(), "Loading non-existent should return empty id");
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
                CHECK(sn.code.isEmpty(), "getAllSnippets(false) should not load code");
                foundNoCode = true;
                break;
            }
        }
        CHECK(foundNoCode, "Snippet should be found in getAllSnippets(false)");
        qDebug() << "PASS: Test 8a - getAllSnippets(false) does not load code";

        QList<Snippet> withCode = mgr.getAllSnippets(true);
        bool foundWithCode = false;
        for (const auto &sn : withCode) {
            if (sn.id == id) {
                CHECK(sn.code == s.code, "getAllSnippets(true) code should match");
                foundWithCode = true;
                break;
            }
        }
        CHECK(foundWithCode, "Snippet should be found in getAllSnippets(true)");
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
        CHECK(reloaded.code == s.code, "renameCategory should preserve code");
        CHECK(reloaded.category == "test/renamed", "renameCategory should change category");
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

        bool exportOk = mgr.exportSnippetZip(id, zipPath);
        CHECK(exportOk, "Export snippet should succeed");
        CHECK(QFile::exists(zipPath), "Archive file should exist after export");
        qDebug() << "PASS: Test 10a - Export snippet to archive";

        QStringList importedIds = mgr.importSnippetsZip(zipPath);
        CHECK(!importedIds.isEmpty(), "Import should return non-empty list");
        qDebug() << "PASS: Test 10b - Import snippet from archive";

        if (importedIds.isEmpty()) {
            qDebug() << "SKIP: Test 10c - Cannot test imported data (import failed)";
        } else {
            QString newId = importedIds.first();
            CHECK(mgr.snippetExists(newId), "Imported snippet should exist");
            CHECK(!mgr.isPresetId(newId), "Imported snippet should not be a preset");
            Snippet imported = mgr.loadSnippet(newId);
            CHECK(imported.name == s.name, "Imported name should match");
            CHECK(imported.description == s.description, "Imported description should match");
            CHECK(imported.code == s.code, "Imported code should match");
            CHECK(imported.tags == s.tags, "Imported tags should match");
            qDebug() << "PASS: Test 10c - Imported data matches original";
            mgr.deleteSnippet(newId);
        }

        QFile::remove(zipPath);
        mgr.deleteSnippet(id);
    }

    // Test 11: compileCommand field serialization/deserialization
    {
        QString id = mgr.createSnippet("CompileCmd Test", "test");
        CHECK(!id.isEmpty(), "Should create snippet for compileCommand test");

        Snippet s = mgr.loadSnippet(id);
        CHECK(s.compileCommand.isEmpty(), "Default compileCommand should be empty");

        s.compileCommand = "lualatex -interaction=nonstopmode -shell-escape";
        bool saved = mgr.saveSnippet(s);
        CHECK(saved, "Save with compileCommand should succeed");

        Snippet s2 = mgr.loadSnippet(id);
        CHECK(s2.compileCommand == "lualatex -interaction=nonstopmode -shell-escape",
              "compileCommand should survive JSON roundtrip");
        qDebug() << "PASS: Test 11 - compileCommand JSON serialization";

        mgr.deleteSnippet(id);
    }

    // Test 12: compileCommand with special characters
    {
        QString id = mgr.createSnippet("CompileCmd Special", "test");
        Snippet s = mgr.loadSnippet(id);
        s.compileCommand = "/usr/bin/xelatex -interaction=nonstopmode -halt-on-error -shell-escape";
        mgr.saveSnippet(s);

        Snippet s2 = mgr.loadSnippet(id);
        CHECK(s2.compileCommand == "/usr/bin/xelatex -interaction=nonstopmode -halt-on-error -shell-escape",
              "compileCommand with path should survive roundtrip");
        qDebug() << "PASS: Test 12 - compileCommand with full path";

        mgr.deleteSnippet(id);
    }

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) FAILED!";
        return 1;
    }

    qDebug() << "\nAll tests passed!";
    return 0;
}
