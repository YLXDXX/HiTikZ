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

    // Test 13: sortOrder field serialization/deserialization
    {
        QString id = mgr.createSnippet("SortOrder Test", "test/order");
        CHECK(!id.isEmpty(), "Should create snippet for sortOrder test");

        Snippet s = mgr.loadSnippet(id);
        CHECK(s.sortOrder == 0, "Default sortOrder should be 0");

        s.sortOrder = 42;
        mgr.saveSnippet(s);
        Snippet s2 = mgr.loadSnippet(id);
        CHECK(s2.sortOrder == 42, "sortOrder should survive JSON roundtrip");
        qDebug() << "PASS: Test 13 - sortOrder JSON serialization";

        mgr.deleteSnippet(id);
    }

    // Test 14: reorderSnippets
    {
        QString id1 = mgr.createSnippet("Reorder A", "test/reorder");
        QString id2 = mgr.createSnippet("Reorder B", "test/reorder");
        QString id3 = mgr.createSnippet("Reorder C", "test/reorder");

        double o1 = mgr.loadSnippet(id1).sortOrder;
        double o2 = mgr.loadSnippet(id2).sortOrder;
        double o3 = mgr.loadSnippet(id3).sortOrder;
        CHECK(o3 < o2 && o2 < o1, "New snippets should be at front (descending sortOrders)");

        // Reorder: C, A, B
        QStringList orderedIds = {id3, id1, id2};
        mgr.reorderSnippets(orderedIds);

        CHECK(mgr.loadSnippet(id3).sortOrder == 0, "First item sortOrder should be 0");
        CHECK(mgr.loadSnippet(id1).sortOrder == 1, "Second item sortOrder should be 1");
        CHECK(mgr.loadSnippet(id2).sortOrder == 2, "Third item sortOrder should be 2");

        // Reorder again: A, C, B
        mgr.reorderSnippets({id1, id3, id2});
        CHECK(mgr.loadSnippet(id1).sortOrder == 0, "After reorder: A should be 0");
        CHECK(mgr.loadSnippet(id3).sortOrder == 1, "After reorder: C should be 1");
        CHECK(mgr.loadSnippet(id2).sortOrder == 2, "After reorder: B should be 2");

        qDebug() << "PASS: Test 14 - reorderSnippets";

        mgr.deleteSnippet(id1);
        mgr.deleteSnippet(id2);
        mgr.deleteSnippet(id3);
    }

    // Test 15: Category order save/load
    {
        QStringList order1 = {"math", "physics", "chemistry"};
        mgr.saveCategoryOrder(order1);

        QStringList loaded = mgr.loadCategoryOrder();
        CHECK(loaded.size() == 3, "Loaded category order should have 3 items");
        CHECK(loaded[0] == "math", "First category should be math");
        CHECK(loaded[1] == "physics", "Second category should be physics");
        CHECK(loaded[2] == "chemistry", "Third category should be chemistry");

        QStringList order2 = {"chemistry", "math"};
        mgr.saveCategoryOrder(order2);
        loaded = mgr.loadCategoryOrder();
        CHECK(loaded.size() == 2, "Updated order should have 2 items");
        CHECK(loaded[0] == "chemistry", "Reordered: chemistry should be first");
        CHECK(loaded[1] == "math", "Reordered: math should be second");

        qDebug() << "PASS: Test 15 - Category order save/load";
    }

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) FAILED!";
        return 1;
    }

    qDebug() << "\nAll tests passed!";
    return 0;
}
