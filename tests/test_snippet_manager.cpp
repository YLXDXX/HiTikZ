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

    qDebug() << "\nAll tests passed!";
    return 0;
}
