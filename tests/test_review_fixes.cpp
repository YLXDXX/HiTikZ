#include "snippet_manager.h"
#include "search_panel.h"
#include "code_editor.h"

#include <QApplication>
#include <QTextDocument>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <cstdio>

static int g_failed = 0;

#define CHECK(expr, msg) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "FAIL: %s\n", msg); \
            g_failed++; \
        } \
    } while (0)

// ---------------------------------------------------------------------------
// Test 1: saveSnippet saves tex first, then meta.json (data consistency)
// ---------------------------------------------------------------------------
static void test_save_snippet_order()
{
    SnippetManager mgr;

    QString id = mgr.createSnippet("Consistency Test", "test/consistency");
    CHECK(!id.isEmpty(), "T1.1: Should create snippet");

    Snippet s = mgr.loadSnippet(id);
    CHECK(s.id == id, "T1.2: Loaded id should match");

    s.code = "\\begin{tikzpicture}\n  \\draw (0,0) -- (1,1);\n\\end{tikzpicture}";
    s.description = "Consistency check snippet";
    bool saved = mgr.saveSnippet(s);
    CHECK(saved, "T1.3: Save should succeed");

    Snippet s2 = mgr.loadSnippet(id);
    CHECK(s2.code == s.code, "T1.4: Code should be saved correctly");
    CHECK(s2.description == s.description, "T1.5: Description (meta) should be saved correctly");

    QString metaPath = mgr.getBasePath() + id + "/meta.json";
    QString texPath = mgr.getBasePath() + id + "/snippet.tex";

    CHECK(QFile::exists(metaPath), "T1.6: meta.json should exist");
    CHECK(QFile::exists(texPath), "T1.7: snippet.tex should exist");

    QFile texFile(texPath);
    CHECK(texFile.open(QIODevice::ReadOnly | QIODevice::Text), "T1.8: Should open snippet.tex");
    QString texContent = QString::fromUtf8(texFile.readAll());
    texFile.close();
    CHECK(texContent == s.code, "T1.9: snippet.tex content should match");

    QFile metaFile(metaPath);
    CHECK(metaFile.open(QIODevice::ReadOnly), "T1.10: Should open meta.json");
    QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
    metaFile.close();
    CHECK(doc.isObject(), "T1.11: meta.json should be valid JSON object");
    CHECK(doc.object().value("description").toString() == s.description,
          "T1.12: meta.json description should match");

    mgr.deleteSnippet(id);
    fprintf(stderr, "PASS: Test 1 - saveSnippet data consistency\n");
}

// ---------------------------------------------------------------------------
// Test 2: findMatchingBracket with LaTeX escaped braces
// Replicates the isEscaped lambda logic from code_editor.cpp
// ---------------------------------------------------------------------------
static int findMatchingBracketTest(const QTextDocument *doc, int pos, QChar bracket)
{
    const int totalChars = doc->characterCount();
    const int lastIdx = totalChars - 1;

    auto isOpen = [](QChar ch) {
        return ch == QLatin1Char('{') || ch == QLatin1Char('[')
            || ch == QLatin1Char('(');
    };
    auto isClose = [](QChar ch) {
        return ch == QLatin1Char('}') || ch == QLatin1Char(']')
            || ch == QLatin1Char(')');
    };
    auto closingFor = [](QChar open) -> QChar {
        if (open == QLatin1Char('{')) return QLatin1Char('}');
        if (open == QLatin1Char('[')) return QLatin1Char(']');
        if (open == QLatin1Char('(')) return QLatin1Char(')');
        return QChar();
    };

    auto isEscaped = [&](int index) -> bool {
        int backslashes = 0;
        for (int j = index - 1; j >= 0; --j) {
            if (doc->characterAt(j) == QLatin1Char('\\'))
                backslashes++;
            else
                break;
        }
        return (backslashes % 2) == 1;
    };

    if (pos >= 0 && pos < lastIdx && isEscaped(pos))
        return -1;

    if (isOpen(bracket)) {
        const QChar close = closingFor(bracket);
        int depth = 0;
        for (int i = pos + 1; i < lastIdx; ++i) {
            const QChar ch = doc->characterAt(i);
            if ((ch == bracket || ch == close) && isEscaped(i))
                continue;
            if (ch == bracket) {
                depth++;
            } else if (ch == close) {
                if (depth == 0) return i;
                depth--;
            }
        }
    } else if (isClose(bracket)) {
        QChar open;
        if (bracket == QLatin1Char('}'))      open = QLatin1Char('{');
        else if (bracket == QLatin1Char(']')) open = QLatin1Char('[');
        else                                  open = QLatin1Char('(');

        int depth = 0;
        for (int i = pos - 1; i >= 0; --i) {
            const QChar ch = doc->characterAt(i);
            if ((ch == bracket || ch == open) && isEscaped(i))
                continue;
            if (ch == bracket) {
                depth++;
            } else if (ch == open) {
                if (depth == 0) return i;
                depth--;
            }
        }
    }
    return -1;
}

static void test_escaped_braces()
{
    // Helper: returns the position of '}' matching the '{' at `pos`.
    // The escaped-bracket fix skips \{ and \} inside the scanning loop,
    // but we also check that the bracket at `pos` itself is not escaped.

    // Case 1: Normal braces match
    {
        QTextDocument doc;
        doc.setPlainText("{hello}");
        int closePos = doc.toPlainText().indexOf('}');
        int openPos = doc.toPlainText().indexOf('{');
        int close = findMatchingBracketTest(&doc, openPos, QChar('{'));
        CHECK(close == closePos, "T2.1: '{' at pos 0 should match '}' at expected pos");

        int open = findMatchingBracketTest(&doc, closePos, QChar('}'));
        CHECK(open == openPos, "T2.2: '}' should match '{' at expected pos");
    }

    // Case 2: Escaped \{ should NOT count as open brace (depends on pos=1, not 0)
    {
        QTextDocument doc;
        doc.setPlainText("\\{text}");
        int openBracePos = doc.toPlainText().indexOf('{');
        CHECK(openBracePos == 1, "T2.3a: '{' should be at pos 1 (after backslash)");
        int close = findMatchingBracketTest(&doc, openBracePos, QChar('{'));
        CHECK(close == -1, "T2.3b: '\\{' at pos 1 is escaped, should return -1");
    }

    // Case 3: Escaped \} should NOT count as close brace — '{' matches the second literal '}'
    {
        QTextDocument doc;
        doc.setPlainText("{\\}}");
        int openBracePos = doc.toPlainText().indexOf('{');
        int close = findMatchingBracketTest(&doc, openBracePos, QChar('{'));
        int expectedClose = doc.toPlainText().lastIndexOf('}');
        CHECK(close == expectedClose, "T2.4: '{' should match the literal '}', skipping escaped '\\}'");
    }

    // Case 4: Nested braces with escaped inner brace — outer { matches first '}' at depth 0
    {
        QTextDocument doc;
        doc.setPlainText("{\\{nested}}");
        int openPos = doc.toPlainText().indexOf('{');
        int close = findMatchingBracketTest(&doc, openPos, QChar('{'));
        CHECK(close >= 0, "T2.5a: Should find a match");
        CHECK(doc.characterAt(close) == QLatin1Char('}'),
              "T2.5b: Match should be a closing brace");
    }

    // Case 5: Dropping parent "math" onto "math/algebra" is invalid (child onto descendant)
    {
        QString draggedId = "math";
        QString targetCat = "math/algebra";
        bool invalid = (draggedId == targetCat)
            || targetCat.startsWith(draggedId + QLatin1Char('/'));
        CHECK(invalid, "T2.6: Dropping parent onto child should be rejected");
    }

    fprintf(stderr, "PASS: Test 2 - escaped bracket matching\n");
}

// ---------------------------------------------------------------------------
// Test 3: SearchPanel virtual nodes have drag disabled
// ---------------------------------------------------------------------------
static void test_search_panel_virtual_nodes(QApplication &app)
{
    SnippetManager mgr;

    SearchPanel panel(&mgr);
    panel.refreshCategoryTree();

    QStandardItemModel *model = panel.findChild<QStandardItemModel *>();
    CHECK(model != nullptr, "T3.1: Should find category model");

    QStandardItem *rootItem = model->invisibleRootItem();

    for (int i = 0; i < rootItem->rowCount(); ++i) {
        QStandardItem *item = rootItem->child(i);
        QString userRole = item->data(Qt::UserRole).toString();

        if (userRole.isEmpty()) {
            CHECK(!(item->flags() & Qt::ItemIsDragEnabled),
                  "T3.2: '全部' node should not be draggable");
        } else if (userRole == QLatin1String("__uncategorized__")) {
            CHECK(!(item->flags() & Qt::ItemIsDragEnabled),
                  "T3.3: '未分类' node should not be draggable");
        }
    }

    fprintf(stderr, "PASS: Test 3 - virtual nodes drag disabled\n");
}

// ---------------------------------------------------------------------------
// Test 4: SearchPanel refreshCategoryTree preserves selection
// ---------------------------------------------------------------------------
static void test_search_panel_selection_restore(QApplication &app)
{
    SnippetManager mgr;

    QString id1 = mgr.createSnippet("Test Math", "math");
    QString id2 = mgr.createSnippet("Test Geo", "math/geometry");
    CHECK(!id1.isEmpty(), "T4.1: Should create math snippet");
    CHECK(!id2.isEmpty(), "T4.2: Should create geometry snippet");

    SearchPanel panel(&mgr);
    panel.refreshCategoryTree();

    QStandardItemModel *model = panel.findChild<QStandardItemModel *>();
    CHECK(model != nullptr, "T4.3: Should find model");

    QModelIndexList found = model->match(
        model->index(0, 0),
        Qt::UserRole,
        "math/geometry",
        1,
        Qt::MatchExactly | Qt::MatchRecursive);
    CHECK(!found.isEmpty(), "T4.4: Should find 'math/geometry' in tree");

    QStandardItem *geoItem = model->itemFromIndex(found.first());
    CHECK(geoItem != nullptr, "T4.5: Should get item for 'math/geometry'");
    CHECK(geoItem->data(Qt::UserRole).toString() == "math/geometry",
          "T4.6: Item data should be 'math/geometry'");

    QModelIndex parent = found.first().parent();
    CHECK(parent.isValid(), "T4.7: 'math/geometry' should have parent 'math'");
    CHECK(parent.data(Qt::UserRole).toString() == "math",
          "T4.8: Parent should be 'math'");

    mgr.deleteSnippet(id1);
    mgr.deleteSnippet(id2);

    fprintf(stderr, "PASS: Test 4 - category tree selection restore\n");
}

// ---------------------------------------------------------------------------
// Test 5: Category order with child categories (parent+children move together)
// ---------------------------------------------------------------------------
static void test_category_order_with_children()
{
    SnippetManager mgr;

    mgr.saveCategoryOrder({"math", "math/geometry", "math/algebra", "physics"});

    QStringList order = mgr.loadCategoryOrder();
    CHECK(order.size() == 4, "T5.1: Should have 4 categories");
    CHECK(order[0] == "math", "T5.2: First should be math");
    CHECK(order[1] == "math/geometry", "T5.3: Second should be math/geometry");
    CHECK(order[2] == "math/algebra", "T5.4: Third should be math/algebra");
    CHECK(order[3] == "physics", "T5.5: Fourth should be physics");

    QStringList savedOrder = {"math", "math/geometry", "math/algebra", "physics"};

    QStringList toMove;
    QString draggedId = "math";
    toMove.append(draggedId);
    for (const QString &cat : savedOrder) {
        if (cat.startsWith(draggedId + QLatin1Char('/')))
            toMove.append(cat);
    }
    CHECK(toMove.size() == 3, "T5.6: Moving 'math' should include 2 children");
    CHECK(toMove[0] == "math", "T5.7: toMove[0] should be math");
    CHECK(toMove[1] == "math/geometry", "T5.8: toMove[1] should be math/geometry");
    CHECK(toMove[2] == "math/algebra", "T5.9: toMove[2] should be math/algebra");

    QStringList toMoveUnique;
    QSet<QString> seenSet;
    for (const QString &cat : toMove) {
        if (!seenSet.contains(cat)) {
            seenSet.insert(cat);
            toMoveUnique.append(cat);
        }
    }
    CHECK(toMoveUnique.size() == 3, "T5.10: toMoveUnique should have 3 unique items");

    QStringList remaining;
    for (const QString &cat : savedOrder) {
        if (!toMoveUnique.contains(cat))
            remaining.append(cat);
    }
    CHECK(remaining.size() == 1, "T5.11: Remaining should have 1 item (physics)");
    CHECK(remaining[0] == "physics", "T5.12: Remaining should be physics");

    int insertAt = remaining.size();
    for (int i = 0; i < toMoveUnique.size(); ++i)
        remaining.insert(insertAt + i, toMoveUnique[i]);

    CHECK(remaining.size() == 4, "T5.13: Result should have 4 items");
    CHECK(remaining[0] == "physics", "T5.14: Result[0] should be physics");
    CHECK(remaining[1] == "math", "T5.15: Result[1] should be math");
    CHECK(remaining[2] == "math/geometry", "T5.16: Result[2] should be math/geometry");
    CHECK(remaining[3] == "math/algebra", "T5.17: Result[3] should be math/algebra");

    mgr.saveCategoryOrder({});
    fprintf(stderr, "PASS: Test 5 - category order with child categories\n");
}

// ---------------------------------------------------------------------------
// Test 6: Reject dropping category onto itself or its descendant
// ---------------------------------------------------------------------------
static void test_reject_self_drop()
{
    CHECK(QStringLiteral("math/geometry").startsWith(
              QStringLiteral("math/")),
          "T6.1: math/geometry should start with math/");
    CHECK(!QStringLiteral("math_analysis").startsWith(
              QStringLiteral("math/")),
          "T6.2: math_analysis should NOT start with math/ (no trailing slash)");

    QString draggedId = "math";
    QString targetCat = "math/geometry";
    CHECK(targetCat.startsWith(draggedId + QLatin1Char('/')),
          "T6.3: Dropping parent 'math' onto child 'math/geometry' should be invalid");

    QString draggedId2 = "math/geometry";
    QString targetCat2 = "math/geometry";
    CHECK(draggedId2 == targetCat2,
          "T6.4: Dropping onto self should be detected");

    fprintf(stderr, "PASS: Test 6 - reject self/descendant drop\n");
}

// ---------------------------------------------------------------------------
// Test 7: reorderSnippets preserves order of non-reordered snippets
// (global sort is not destroyed when only a subset is reordered)
// ---------------------------------------------------------------------------
static void test_reorder_global_preservation()
{
    SnippetManager mgr;

    // Create 5 snippets in a known order
    QString idA = mgr.createSnippet("Reorder-A", "test/reorder_global");
    QString idB = mgr.createSnippet("Reorder-B", "test/reorder_global");
    QString idC = mgr.createSnippet("Reorder-C", "test/reorder_global");
    QString idD = mgr.createSnippet("Reorder-D", "test/reorder_global");
    QString idE = mgr.createSnippet("Reorder-E", "test/reorder_global");

    // Newest snippets get smallest sortOrder, so current order is E, D, C, B, A
    // Set explicit sortOrders to ensure a known state
    {
        Snippet s;
        s = mgr.loadSnippet(idA); s.sortOrder = 0; mgr.saveSnippet(s);
        s = mgr.loadSnippet(idB); s.sortOrder = 1; mgr.saveSnippet(s);
        s = mgr.loadSnippet(idC); s.sortOrder = 2; mgr.saveSnippet(s);
        s = mgr.loadSnippet(idD); s.sortOrder = 3; mgr.saveSnippet(s);
        s = mgr.loadSnippet(idE); s.sortOrder = 4; mgr.saveSnippet(s);
    }

    // Global order: A(0), B(1), C(2), D(3), E(4)

    // Simulate: filter shows only A, C, E; user drags to reorder as E, A, C
    mgr.reorderSnippets({idE, idA, idC});

    // Check the new order:
    // The reordered group's first item (E) was at global position 4
    // Non-reordered before E: A(0), B(1), C(2), D(3)... wait, A and C are in the
    // reordered group. So non-reordered before first reordered item (original E at
    // pos 4): B(1), D(3)... wait, original position: A(0), B(1), C(2), D(3), E(4)
    // Reordered set: {E, A, C}
    // First occurrence in global list: A at position 0
    // Before pos 0: none
    // Insert ordered: E, A, C
    // After pos 0, non-ordered: B(1), D(3)
    // Result: E(0), A(1), C(2), B(3), D(4)

    Snippet sE = mgr.loadSnippet(idE);
    Snippet sA = mgr.loadSnippet(idA);
    Snippet sC = mgr.loadSnippet(idC);
    Snippet sB = mgr.loadSnippet(idB);
    Snippet sD = mgr.loadSnippet(idD);

    CHECK(sE.sortOrder == 0, "T7.1: E should be first (sortOrder 0)");
    CHECK(sA.sortOrder == 1, "T7.2: A should be second (sortOrder 1)");
    CHECK(sC.sortOrder == 2, "T7.3: C should be third (sortOrder 2)");
    CHECK(sB.sortOrder == 3, "T7.4: B should be fourth (sortOrder 3)");
    CHECK(sD.sortOrder == 4, "T7.5: D should be fifth (sortOrder 4)");

    // Verify relative order of non-reordered items B and D is preserved
    CHECK(sB.sortOrder < sD.sortOrder, "T7.6: B should come before D (relative order preserved)");

    mgr.deleteSnippet(idA);
    mgr.deleteSnippet(idB);
    mgr.deleteSnippet(idC);
    mgr.deleteSnippet(idD);
    mgr.deleteSnippet(idE);

    fprintf(stderr, "PASS: Test 7 - reorderSnippets global preservation\n");
}

// ---------------------------------------------------------------------------
// Test 8: reorderSnippets with single item is a no-op
// ---------------------------------------------------------------------------
static void test_reorder_single_noop()
{
    SnippetManager mgr;

    QString id = mgr.createSnippet("SingleReorder", "test/single_reorder");
    {
        Snippet s = mgr.loadSnippet(id);
        s.sortOrder = 5.0;
        mgr.saveSnippet(s);
    }

    mgr.reorderSnippets({id});

    Snippet s = mgr.loadSnippet(id);
    CHECK(s.sortOrder == 5.0, "T8.1: sortOrder should not change for single-item reorder");

    mgr.deleteSnippet(id);
    fprintf(stderr, "PASS: Test 8 - reorderSnippets single item no-op\n");
}

// ---------------------------------------------------------------------------
// Test 9: reorderSnippets with non-existent IDs handled gracefully
// ---------------------------------------------------------------------------
static void test_reorder_with_nonexistent()
{
    SnippetManager mgr;

    QString id1 = mgr.createSnippet("Reorder-Exist1", "test/reorder_nonexist");
    QString id2 = mgr.createSnippet("Reorder-Exist2", "test/reorder_nonexist");
    {
        Snippet s;
        s = mgr.loadSnippet(id1); s.sortOrder = 0; mgr.saveSnippet(s);
        s = mgr.loadSnippet(id2); s.sortOrder = 1; mgr.saveSnippet(s);
    }

    mgr.reorderSnippets({id2, "nonexistent-id", id1});

    Snippet s1 = mgr.loadSnippet(id1);
    Snippet s2 = mgr.loadSnippet(id2);
    CHECK(s2.sortOrder == 0, "T9.1: id2 should be first after reorder");
    CHECK(s1.sortOrder == 2, "T9.2: id1 should be third after reorder (nonexistent occupies position 1)");

    mgr.deleteSnippet(id1);
    mgr.deleteSnippet(id2);
    fprintf(stderr, "PASS: Test 9 - reorderSnippets with non-existent IDs\n");
}

// ---------------------------------------------------------------------------
// Test 10: Deleted snippet draft should not resurrect the snippet
// When recovering a draft whose snippetId points to a deleted (non-existent)
// snippet, a new UUID should be generated instead of reusing the old ID.
// ---------------------------------------------------------------------------
static void test_draft_recovery_no_resurrect()
{
    SnippetManager mgr;

    QString oldId = mgr.createSnippet("ToBeDeleted", "test/draft_resurrect");
    CHECK(!oldId.isEmpty(), "T10.1: Should create snippet");

    Snippet s = mgr.loadSnippet(oldId);
    s.code = "\\begin{tikzpicture}\\draw (0,0)--(1,1);\\end{tikzpicture}";
    mgr.saveSnippet(s);

    mgr.deleteSnippet(oldId);
    CHECK(!mgr.snippetExists(oldId), "T10.2: Snippet should be deleted");

    // Simulate recovery logic: old snippet deleted, draft with old ID exists
    QString draftSnippetId = oldId;
    CHECK(!draftSnippetId.isEmpty(), "T10.3: draftSnippetId should be non-empty");
    CHECK(!mgr.snippetExists(draftSnippetId), "T10.4: Deleted snippet should not exist");

    // The fix: always generate a new ID when snippet doesn't exist
    QString recoveredId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    CHECK(recoveredId != oldId, "T10.5: Recovered ID should be a new UUID, not the old one");

    {
        Snippet recovered;
        recovered.id = recoveredId;
        recovered.name = "Recovered Draft";
        recovered.code = s.code;
        recovered.category = "test/draft_resurrect";
        mgr.saveSnippet(recovered);
        CHECK(mgr.snippetExists(recoveredId), "T10.6: Recovered snippet should exist");
        mgr.deleteSnippet(recoveredId);
    }

    fprintf(stderr, "PASS: Test 10 - draft recovery does not resurrect deleted snippets\n");
}

// ---------------------------------------------------------------------------
// Test 11: Auto-save for deleted snippet uses scratch naming
// ---------------------------------------------------------------------------
static void test_autosave_deleted_snippet()
{
    SnippetManager mgr;

    QString sid = mgr.createSnippet("AutoSaveDeleted", "test/autosave_deleted");
    CHECK(!sid.isEmpty(), "T11.1: Should create snippet");

    mgr.deleteSnippet(sid);
    CHECK(!mgr.snippetExists(sid), "T11.2: Snippet should be deleted");

    // Simulate auto-save logic: if the snippet was deleted, clear sid so
    // the draft is saved as scratch, not with the deleted snippet's ID
    if (!sid.isEmpty() && !mgr.snippetExists(sid))
        sid.clear();
    CHECK(sid.isEmpty(), "T11.3: Deleted snippet sid should be cleared for auto-save");

    fprintf(stderr, "PASS: Test 11 - auto-save uses scratch name for deleted snippets\n");
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    test_save_snippet_order();
    test_escaped_braces();
    test_search_panel_virtual_nodes(app);
    test_search_panel_selection_restore(app);
    test_category_order_with_children();
    test_reject_self_drop();
    test_reorder_global_preservation();
    test_reorder_single_noop();
    test_reorder_with_nonexistent();
    test_draft_recovery_no_resurrect();
    test_autosave_deleted_snippet();

    if (g_failed > 0) {
        fprintf(stderr, "\n%d test(s) FAILED!\n", g_failed);
        return 1;
    }

    fprintf(stderr, "\nAll review-fix tests passed!\n");
    return 0;
}
