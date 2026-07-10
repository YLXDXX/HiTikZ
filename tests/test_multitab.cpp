#include <QApplication>
#include <QTabWidget>
#include <QTabBar>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "../src/mainwindow.h"
#include "../src/snippet_manager.h"
#include "../src/search_panel.h"
#include "../src/code_editor.h"
#include "../src/tikz_document_state.h"
#include "../src/tikz_completer.h"
#include <QLineEdit>

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL: %s\n", msg); \
            g_testsFailed++; \
        } else { \
            g_testsPassed++; \
        } \
    } while (0)

static void test_initial_state(QTabWidget *tabWidget)
{
    TEST_ASSERT(tabWidget != nullptr, "tabWidget should exist");
    if (!tabWidget) return;
    TEST_ASSERT(tabWidget->count() == 0, "initial tab count should be 0");
    TEST_ASSERT(tabWidget->currentIndex() == -1, "no tab should be current initially");
}

static void test_create_tab(SnippetManager *snippetMgr, SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    TEST_ASSERT(snippetMgr != nullptr, "snippetMgr should exist");
    TEST_ASSERT(searchPanel != nullptr, "searchPanel should exist");
    if (!snippetMgr || !searchPanel) return;

    QString id1 = snippetMgr->createSnippet("Test Snippet 1", "test/category");
    TEST_ASSERT(!id1.isEmpty(), "should create snippet 1");

    emit searchPanel->snippetSelected(id1);
    QApplication::processEvents();

    TEST_ASSERT(tabWidget->count() == 1, "tab count should be 1 after loading snippet");
    if (tabWidget->count() > 0) {
        fprintf(stderr, "  tab title: %s\n", tabWidget->tabText(0).toUtf8().constData());
        QString tabSid = tabWidget->tabBar()->tabData(0).toString();
        TEST_ASSERT(tabSid == id1, "tab data should match snippet id");
    }
}

static void test_duplicate_snippet_not_reopened(SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    QString id1;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QString sid = tabWidget->tabBar()->tabData(i).toString();
        if (!sid.isEmpty()) {
            id1 = sid;
            break;
        }
    }
    TEST_ASSERT(!id1.isEmpty(), "should have an existing snippet id");

    int tabCountBefore = tabWidget->count();
    emit searchPanel->snippetSelected(id1);
    QApplication::processEvents();

    TEST_ASSERT(tabWidget->count() == tabCountBefore,
        "tab count should not increase when opening already open snippet");
}

static void test_create_second_tab(SnippetManager *snippetMgr, SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    QString id2 = snippetMgr->createSnippet("Test Snippet 2", "test/category2");
    TEST_ASSERT(!id2.isEmpty(), "should create snippet 2");

    int tabCountBefore = tabWidget->count();
    emit searchPanel->snippetSelected(id2);
    QApplication::processEvents();

    TEST_ASSERT(tabWidget->count() == tabCountBefore + 1,
        "tab count should increase by 1 when opening new snippet");

    int lastIdx = tabWidget->count() - 1;
    TEST_ASSERT(tabWidget->tabBar()->tabData(lastIdx).toString() == id2,
        "second tab should have correct snippet id");
    TEST_ASSERT(tabWidget->currentIndex() == lastIdx,
        "second tab should be the current tab");
}

static void test_tab_switching(QTabWidget *tabWidget)
{
    TEST_ASSERT(tabWidget->count() >= 2, "should have at least 2 tabs");
    if (tabWidget->count() < 2) return;

    tabWidget->setCurrentIndex(0);
    QApplication::processEvents();
    TEST_ASSERT(tabWidget->currentIndex() == 0, "should switch to tab 0");

    tabWidget->setCurrentIndex(1);
    QApplication::processEvents();
    TEST_ASSERT(tabWidget->currentIndex() == 1, "should switch to tab 1");
}

static void test_close_tab(QTabWidget *tabWidget)
{
    int countBefore = tabWidget->count();
    TEST_ASSERT(countBefore >= 2, "should have at least 2 tabs to close one");
    if (countBefore < 2) return;

    QWidget *widget = tabWidget->widget(0);
    tabWidget->removeTab(0);
    widget->deleteLater();
    QApplication::processEvents();

    TEST_ASSERT(tabWidget->count() == countBefore - 1,
        "tab count should decrease by 1 after closing");
}

static void test_reopen_snippet_after_close(SnippetManager *snippetMgr, SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    QString id = snippetMgr->createSnippet("Reopen Test", "test/reopen");
    TEST_ASSERT(!id.isEmpty(), "should create reopen test snippet");

    emit searchPanel->snippetSelected(id);
    QApplication::processEvents();
    TEST_ASSERT(tabWidget->count() >= 1, "tab should be created for reopen test snippet");

    int tabIdx = -1;
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabBar()->tabData(i).toString() == id) {
            tabIdx = i;
            break;
        }
    }
    TEST_ASSERT(tabIdx >= 0, "should find tab for reopen test snippet");

    if (tabIdx >= 0) {
        QWidget *widget = tabWidget->widget(tabIdx);
        tabWidget->removeTab(tabIdx);
        widget->deleteLater();
        QApplication::processEvents();
    }

    int countBefore = tabWidget->count();

    emit searchPanel->snippetSelected(id);
    QApplication::processEvents();

    bool reopened = false;
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabBar()->tabData(i).toString() == id) {
            reopened = true;
            break;
        }
    }
    TEST_ASSERT(reopened, "snippet should be reopened after closing tab");
    TEST_ASSERT(tabWidget->count() > countBefore,
        "tab count should increase after reopening snippet");

    snippetMgr->deleteSnippet(id);
}

static void test_snippet_modified_sync(SnippetManager *snippetMgr, SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    QString id = snippetMgr->createSnippet("Sync Test", "test/sync");
    TEST_ASSERT(!id.isEmpty(), "should create sync test snippet");

    Snippet s = snippetMgr->loadSnippet(id);
    s.name = "Initial Name";
    s.description = "Initial Description";
    QStringList tags;
    tags << "tag1" << "tag2";
    s.tags = tags;
    s.packages = "testpkg";
    s.tikzLibraries = "calc";
    s.code = "\\draw (0,0) -- (1,1);";
    snippetMgr->saveSnippet(s);

    emit searchPanel->snippetSelected(id);
    QApplication::processEvents();

    int tabIdx = -1;
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabBar()->tabData(i).toString() == id) {
            tabIdx = i;
            break;
        }
    }
    TEST_ASSERT(tabIdx >= 0, "should find tab for sync test snippet");
    TEST_ASSERT(tabWidget->currentIndex() == tabIdx, "sync test tab should be current");

    Snippet modified = snippetMgr->loadSnippet(id);
    modified.name = "Updated Name";
    modified.description = "Updated Description";
    QStringList newTags;
    newTags << "newtag";
    modified.tags = newTags;
    modified.packages = "newpkg";
    modified.tikzLibraries = "patterns";
    modified.code = modified.code;
    snippetMgr->saveSnippet(modified);

    QApplication::processEvents();

    Snippet loaded = snippetMgr->loadSnippet(id);
    TEST_ASSERT(loaded.name == "Updated Name",
        "snippet name should be updated in storage");
    TEST_ASSERT(loaded.packages == "newpkg",
        "snippet packages should be updated in storage");

    QString tabTitle = tabWidget->tabText(tabIdx);
    TEST_ASSERT(tabTitle.contains("Updated Name"),
        "tab title should reflect updated snippet name");

    snippetMgr->deleteSnippet(id);
}

// Verifies unsaved-change detection considers metadata edits (not just code),
// so closing/exiting won't silently discard metadata changes.
static void test_metadata_dirty_detection(MainWindow *mw, SnippetManager *snippetMgr,
                                          SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    QString id = snippetMgr->createSnippet("Dirty Test", "test/dirty");
    Snippet s = snippetMgr->loadSnippet(id);
    s.description = "Original desc";
    s.code = "\\draw (0,0) -- (1,1);";
    snippetMgr->saveSnippet(s);

    emit searchPanel->snippetSelected(id);
    QApplication::processEvents();

    int idx = -1;
    for (int i = 0; i < tabWidget->count(); ++i)
        if (tabWidget->tabBar()->tabData(i).toString() == id) { idx = i; break; }
    TEST_ASSERT(idx >= 0, "dirty test tab should exist");
    TEST_ASSERT(tabWidget->currentIndex() == idx, "dirty test tab should be current");

    // Freshly loaded, unmodified snippet must be clean.
    TEST_ASSERT(!mw->tabHasUnsavedChanges(idx),
                "freshly loaded snippet should not be dirty");

    // Editing only the description (metadata) must mark the tab dirty.
    QTextEdit *descEdit = mw->findChild<QTextEdit*>(QStringLiteral("metaDescEdit"));
    TEST_ASSERT(descEdit != nullptr, "should find metaDescEdit by object name");
    if (descEdit) {
        descEdit->setPlainText(QStringLiteral("Edited description"));
        QApplication::processEvents();
        TEST_ASSERT(mw->tabHasUnsavedChanges(idx),
                    "metadata-only edit should be detected as unsaved");
        // Restore so it doesn't interfere with later teardown.
        descEdit->setPlainText(QStringLiteral("Original desc"));
        QApplication::processEvents();
        TEST_ASSERT(!mw->tabHasUnsavedChanges(idx),
                    "reverting metadata should clear the dirty state");
    }

    snippetMgr->deleteSnippet(id);
}

// Reproduces the bug where switching between tabs (selecting different snippets
// in the left panel's thumbnail list) failed to update the right-panel metadata
// and corrupted tab titles (multiple tabs sharing one title / cross-contaminated
// metadata). Verifies each tab shows its own snippet's metadata and title after
// switching, and that background tab titles are never overwritten.
static void test_tab_metadata_isolation(MainWindow *mw, SnippetManager *snippetMgr,
                                        SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    QString idA = snippetMgr->createSnippet("Alpha Snippet", "test/iso");
    Snippet a = snippetMgr->loadSnippet(idA);
    a.description = "Alpha description";
    a.packages = "alphapkg";
    a.tikzLibraries = "calc";
    a.code = "\\draw (0,0) circle (1);";
    snippetMgr->saveSnippet(a);

    QString idB = snippetMgr->createSnippet("Beta Snippet", "test/iso");
    Snippet b = snippetMgr->loadSnippet(idB);
    b.description = "Beta description";
    b.packages = "betapkg";
    b.tikzLibraries = "patterns";
    b.code = "\\draw (0,0) rectangle (2,2);";
    snippetMgr->saveSnippet(b);

    // Open both snippets in separate tabs.
    emit searchPanel->snippetSelected(idA);
    QApplication::processEvents();
    emit searchPanel->snippetSelected(idB);
    QApplication::processEvents();

    int idxA = -1, idxB = -1;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QString sid = tabWidget->tabBar()->tabData(i).toString();
        if (sid == idA) idxA = i;
        else if (sid == idB) idxB = i;
    }
    TEST_ASSERT(idxA >= 0 && idxB >= 0, "both isolation tabs should exist");
    if (idxA < 0 || idxB < 0) { snippetMgr->deleteSnippet(idA); snippetMgr->deleteSnippet(idB); return; }

    QLineEdit *nameEdit = mw->findChild<QLineEdit*>(QStringLiteral("metaNameEdit"));
    QTextEdit *descEdit = mw->findChild<QTextEdit*>(QStringLiteral("metaDescEdit"));
    QLineEdit *pkgEdit  = mw->findChild<QLineEdit*>(QStringLiteral("metaPackagesEdit"));
    TEST_ASSERT(nameEdit && descEdit && pkgEdit, "metadata widgets should be found");
    if (!nameEdit || !descEdit || !pkgEdit) { snippetMgr->deleteSnippet(idA); snippetMgr->deleteSnippet(idB); return; }

    // Switch to A: right panel must reflect A's metadata.
    tabWidget->setCurrentIndex(idxA);
    QApplication::processEvents();
    TEST_ASSERT(nameEdit->text() == "Alpha Snippet", "name should update to Alpha on switch");
    TEST_ASSERT(descEdit->toPlainText() == "Alpha description", "desc should update to Alpha on switch");
    TEST_ASSERT(pkgEdit->text() == "alphapkg", "packages should update to Alpha on switch");

    // Switch to B: right panel must reflect B's metadata (the reported bug: it
    // used to keep showing the previous snippet's info).
    tabWidget->setCurrentIndex(idxB);
    QApplication::processEvents();
    TEST_ASSERT(nameEdit->text() == "Beta Snippet", "name should update to Beta on switch");
    TEST_ASSERT(descEdit->toPlainText() == "Beta description", "desc should update to Beta on switch");
    TEST_ASSERT(pkgEdit->text() == "betapkg", "packages should update to Beta on switch");

    // Tab titles must remain distinct and correct (the reported bug: titles got
    // duplicated / multiple tabs shared the same title).
    TEST_ASSERT(tabWidget->tabText(idxA) == "Alpha Snippet",
                "tab A title should stay 'Alpha Snippet'");
    TEST_ASSERT(tabWidget->tabText(idxB) == "Beta Snippet",
                "tab B title should stay 'Beta Snippet'");
    TEST_ASSERT(tabWidget->tabText(idxA) != tabWidget->tabText(idxB),
                "tab titles must not collide");

    // Editing metadata on the active tab (B) must NOT leak into the background
    // tab (A) or corrupt its title.
    descEdit->setPlainText(QStringLiteral("Beta edited"));
    QApplication::processEvents();
    TEST_ASSERT(tabWidget->tabText(idxA) == "Alpha Snippet",
                "editing active tab must not alter background tab title");

    tabWidget->setCurrentIndex(idxA);
    QApplication::processEvents();
    TEST_ASSERT(descEdit->toPlainText() == "Alpha description",
                "switching back to A must restore A's unedited description");

    tabWidget->setCurrentIndex(idxB);
    QApplication::processEvents();
    TEST_ASSERT(descEdit->toPlainText() == "Beta edited",
                "B's unsaved edit must survive the round-trip switch");

    snippetMgr->deleteSnippet(idA);
    snippetMgr->deleteSnippet(idB);
}

// Regression: editing the right-panel "TikZ库" (TikZ libraries) field must
// propagate the library into the active editor's document state so completion
// reflects it immediately — without requiring \usetikzlibrary{...} in the code.
// Reproduces the reported bug: adding "through" via the UI field did not enable
// the "circle through" completion (only typing \usetikzlibrary{through} did).
static void test_ui_library_field_activates_completion(
    MainWindow *mw, SnippetManager *snippetMgr,
    SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    QString id = snippetMgr->createSnippet("UILib Test", "test/uilib");
    Snippet s = snippetMgr->loadSnippet(id);
    s.tikzLibraries = QString();   // start with no libraries
    s.code = "\\begin{tikzpicture}\n\\end{tikzpicture}\n";
    snippetMgr->saveSnippet(s);

    emit searchPanel->snippetSelected(id);
    QApplication::processEvents();

    int idx = -1;
    for (int i = 0; i < tabWidget->count(); ++i)
        if (tabWidget->tabBar()->tabData(i).toString() == id) { idx = i; break; }
    TEST_ASSERT(idx >= 0, "UILib test tab should exist");
    if (idx < 0) { snippetMgr->deleteSnippet(id); return; }

    CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(idx));
    TEST_ASSERT(editor != nullptr, "UILib tab should have a CodeEditor");
    QLineEdit *libEdit =
        mw->findChild<QLineEdit*>(QStringLiteral("metaTikzLibrariesEdit"));
    TEST_ASSERT(libEdit != nullptr, "should find metaTikzLibrariesEdit");
    if (!editor || !libEdit) { snippetMgr->deleteSnippet(id); return; }

    // Baseline: without the "through" library, "circle through" must NOT be an
    // active library and thus not offered in the option completion model.
    TEST_ASSERT(!editor->documentState()->activeLibs().contains(QStringLiteral("through")),
                "'through' should not be active before adding it via the UI field");

    // Simulate the user typing "through" into the "TikZ库" field.
    libEdit->setText(QStringLiteral("through"));
    QApplication::processEvents();

    // The edit must now be reflected in the editor's document state.
    TEST_ASSERT(editor->documentState()->activeLibs().contains(QStringLiteral("through")),
                "'through' must be active in doc state after UI field edit");

    // And the "circle through" option must be offered by the completer's
    // bracket/option model (node option context).
    QTextCursor cur = editor->textCursor();
    cur.movePosition(QTextCursor::Start);
    // Position inside the tikzpicture body so env context resolves correctly.
    cur.movePosition(QTextCursor::Down);
    editor->setTextCursor(cur);
    editor->completer()->updateUserModels();
    const QStringList opts =
        editor->completer()->modelWordsForContext(TikzCompleter::TkzCtxBrk);
    TEST_ASSERT(opts.contains(QStringLiteral("circle through")),
                "'circle through' should be offered once 'through' is added via UI");

    // Multiple comma-separated libraries added via the UI must all activate.
    libEdit->setText(QStringLiteral("calc, through"));
    QApplication::processEvents();
    TEST_ASSERT(editor->documentState()->activeLibs().contains(QStringLiteral("calc"))
                && editor->documentState()->activeLibs().contains(QStringLiteral("through")),
                "comma-separated UI libraries should all activate");

    snippetMgr->deleteSnippet(id);
}

static void cleanup_snippets(SnippetManager *mgr)
{
    QList<Snippet> all = mgr->getAllSnippets(true);
    for (const Snippet &s : all) {
        if (s.category.startsWith("test/") || s.name.startsWith("Test ")) {
            mgr->deleteSnippet(s.id);
        }
    }
}

static void test_word_wrap_toggle()
{
    CodeEditor editor;
    editor.setPlainText(QStringLiteral(
        "\\draw (0,0) -- (1,0) -- (1,1) to[in=0,out=180] (0,0) -- cycle; "
        "% a very long line that should wrap when word wrap is enabled"));

    editor.setWordWrap(true);
    TEST_ASSERT(editor.lineWrapMode() == QPlainTextEdit::WidgetWidth,
                "setWordWrap(true) should enable widget-width wrapping");

    editor.setWordWrap(false);
    TEST_ASSERT(editor.lineWrapMode() == QPlainTextEdit::NoWrap,
                "setWordWrap(false) should disable wrapping");

    // Toggling back on must restore wrapping (idempotent, no crash).
    editor.setWordWrap(true);
    TEST_ASSERT(editor.lineWrapMode() == QPlainTextEdit::WidgetWidth,
                "setWordWrap(true) again should re-enable wrapping");
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    QApplication app(argc, argv);
    fprintf(stderr, "=== Testing Multi-Tab Functionality ===\n");

    test_word_wrap_toggle();

    QTabWidget *tabWidget = nullptr;
    SnippetManager *snippetMgr = nullptr;
    SearchPanel *searchPanel = nullptr;

    {
        MainWindow mw;
        tabWidget = mw.findChild<QTabWidget*>();
        snippetMgr = mw.findChild<SnippetManager*>();
        searchPanel = mw.findChild<SearchPanel*>();

        fprintf(stderr, "  tabWidget: %p\n", (void*)tabWidget);
        fprintf(stderr, "  snippetMgr: %p\n", (void*)snippetMgr);
        fprintf(stderr, "  searchPanel: %p\n", (void*)searchPanel);

        test_initial_state(tabWidget);

        mw.show();
        QApplication::processEvents();

        test_create_tab(snippetMgr, searchPanel, tabWidget);
        test_duplicate_snippet_not_reopened(searchPanel, tabWidget);
        test_create_second_tab(snippetMgr, searchPanel, tabWidget);
        test_tab_switching(tabWidget);
        test_close_tab(tabWidget);
        test_reopen_snippet_after_close(snippetMgr, searchPanel, tabWidget);
        test_snippet_modified_sync(snippetMgr, searchPanel, tabWidget);
        test_metadata_dirty_detection(&mw, snippetMgr, searchPanel, tabWidget);

        test_tab_metadata_isolation(&mw, snippetMgr, searchPanel, tabWidget);

        test_ui_library_field_activates_completion(&mw, snippetMgr, searchPanel, tabWidget);

        cleanup_snippets(snippetMgr);
    }

    fprintf(stderr, "\n=== Results ===\n");
    fprintf(stderr, "Passed: %d\n", g_testsPassed);
    fprintf(stderr, "Failed: %d\n", g_testsFailed);

    return g_testsFailed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
