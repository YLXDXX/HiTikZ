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
#include "../src/comma_list_completer.h"
#include <QLineEdit>
#include <QCompleter>
#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QAction>
#include <QTextDocument>
#include <QElapsedTimer>

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

static QAction *findActionByText(MainWindow *mw, const QString &text)
{
    for (QAction *a : mw->findChildren<QAction *>()) {
        if (a->text() == text) return a;
    }
    return nullptr;
}

static void pump_events(int ms)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms)
        QApplication::processEvents(QEventLoop::AllEvents, 20);
}

// Regression: the undo/redo toolbar actions used to be permanently clickable,
// even with a pristine document or no tab at all. They must track the current
// editor's actual history availability.
//
// Two independent failure modes were reported by users:
//   a) freshly opened snippet (real TikZ code, timers elapsed) still shows
//      enabled undo — highlighter/reparse timers tick after the initial
//      setPlainText (blocker lifted) and the user expected the buttons to
//      stay disabled until they manually edit;
//   b) undo/redo do not grey out at the extremities (exhausted undo/redo
//      stack).
static void test_undo_redo_action_state(MainWindow *mw, SnippetManager *snippetMgr,
                                        SearchPanel *searchPanel, QTabWidget *tabWidget)
{
    QAction *undoAct = findActionByText(mw, QStringLiteral("撤销"));
    QAction *redoAct = findActionByText(mw, QStringLiteral("重做"));
    TEST_ASSERT(undoAct && redoAct, "undo/redo toolbar actions exist");
    if (!undoAct || !redoAct) return;

    // --- Real snippet with content (the user scenario) ------------------------
    QString id = snippetMgr->createSnippet("Test UndoRedo", "test/undoredo");
    // Content enough to trigger highlight/reparse timers
    {
        Snippet s = snippetMgr->loadSnippet(id);
        s.code = QStringLiteral("\\begin{tikzpicture}\n"
                                "  \\draw (0,0) circle (1);\n"
                                "  \\fill[red] (1,1) rectangle (2,2);\n"
                                "\\end{tikzpicture}");
        snippetMgr->saveSnippet(s);
    }
    emit searchPanel->snippetSelected(id);
    QApplication::processEvents();

    // Let any highlight / reparse / parse-params timers fire.
    // The setPlainText call inside createNewTab uses a QSignalBlocker on the
    // editor, so the initial content load does not create undo entries. But
    // later timers (highlightCurrentLine via cursor move, reparseDocumentState
    // via m_reparseTimer) must not create undo entries either.
    pump_events(500);

    const bool undoAfterLoad = undoAct->isEnabled();
    if (undoAfterLoad) {
        fprintf(stderr, "FAIL: undo enabled after snippet load (should be disabled)\n");
        g_testsFailed++;
    } else {
        g_testsPassed++;
    }
    TEST_ASSERT(!redoAct->isEnabled(), "redo disabled after snippet load");

    CodeEditor *ed = qobject_cast<CodeEditor *>(tabWidget->currentWidget());
    TEST_ASSERT(ed != nullptr, "current editor exists");
    if (!ed) return;

    // --- User makes an edit → undo enables, redo stays disabled ---------------
    ed->insertPlainText(QStringLiteral("  \\draw (0,0) -- (1,1);\n"));
    QApplication::processEvents();
    TEST_ASSERT(undoAct->isEnabled(), "undo enabled after a real edit");
    TEST_ASSERT(!redoAct->isEnabled(), "redo still disabled after a fresh edit");

    // --- Undo exhaustively → undo disabled, redo enabled -----------------------
    int guard = 0;
    while (ed->document()->isUndoAvailable() && ++guard < 100)
        ed->undo();
    QApplication::processEvents();
    TEST_ASSERT(!undoAct->isEnabled(), "undo disabled once history is exhausted");
    TEST_ASSERT(redoAct->isEnabled(), "redo enabled after undoing");

    // --- Redo exhaustively → redo disabled ------------------------------------
    guard = 0;
    while (ed->document()->isRedoAvailable() && ++guard < 100)
        ed->redo();
    QApplication::processEvents();
    TEST_ASSERT(undoAct->isEnabled(), "undo re-enabled after redoing");
    TEST_ASSERT(!redoAct->isEnabled(), "redo disabled once redo history is exhausted");

    // --- Pristine second tab must not inherit the first tab's state ------------
    QString id2 = snippetMgr->createSnippet("Test UndoRedo Pristine", "test/undoredo");
    emit searchPanel->snippetSelected(id2);
    QApplication::processEvents();
    pump_events(500);
    TEST_ASSERT(!undoAct->isEnabled(), "undo disabled on pristine second tab");
    TEST_ASSERT(!redoAct->isEnabled(), "redo disabled on pristine second tab");

    // --- Switching back restores the edited tab's per-document state ------------
    int firstIdx = -1;
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabBar()->tabData(i).toString() == id) {
            firstIdx = i;
            break;
        }
    }
    TEST_ASSERT(firstIdx >= 0, "edited tab still open");
    if (firstIdx >= 0) {
        tabWidget->setCurrentIndex(firstIdx);
        QApplication::processEvents();
        TEST_ASSERT(undoAct->isEnabled(), "undo state restored after switching back");
        TEST_ASSERT(!redoAct->isEnabled(), "redo state restored after switching back");
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

// Helper: send a single character key press to the editor's keyPressEvent.
static void sendKeyText(CodeEditor *editor, const QString &text, int key = 0)
{
    QKeyEvent ev(QEvent::KeyPress, key, Qt::NoModifier, text);
    QApplication::sendEvent(editor, &ev);
}

// Feature 3: '$' auto-pairs like ( [ {: inserts '$$' with the cursor between,
// wraps a selection as $selection$, skips over a closing '$', and backspace
// removes an empty '$$' pair.
static void test_dollar_autopair()
{
    // Insert '$$' with cursor between the two dollars.
    {
        CodeEditor editor;
        editor.setPlainText(QString());
        editor.moveCursor(QTextCursor::End);
        sendKeyText(&editor, QStringLiteral("$"));
        TEST_ASSERT(editor.toPlainText() == QStringLiteral("$$"),
                    "typing '$' on empty line should insert '$$'");
        // Cursor should sit between the two dollars (position 1).
        TEST_ASSERT(editor.textCursor().position() == 1,
                    "cursor should be between the '$$' pair");
    }

    // Wrap a selection: select "x" then type '$' -> "$x$".
    {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("x"));
        QTextCursor c = editor.textCursor();
        c.setPosition(0);
        c.setPosition(1, QTextCursor::KeepAnchor);
        editor.setTextCursor(c);
        sendKeyText(&editor, QStringLiteral("$"));
        TEST_ASSERT(editor.toPlainText() == QStringLiteral("$x$"),
                    "typing '$' with selection 'x' should wrap to '$x$'");
    }

    // Skip over a closing '$': in "$|$", typing '$' moves past instead of "$$$".
    {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("$$"));
        QTextCursor c = editor.textCursor();
        c.setPosition(1);
        editor.setTextCursor(c);
        sendKeyText(&editor, QStringLiteral("$"));
        TEST_ASSERT(editor.toPlainText() == QStringLiteral("$$"),
                    "typing '$' before an existing '$' should skip, not duplicate");
        TEST_ASSERT(editor.textCursor().position() == 2,
                    "cursor should move past the closing '$'");
    }

    // Backspace inside an empty '$$' pair removes both.
    {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("$$"));
        QTextCursor c = editor.textCursor();
        c.setPosition(1);
        editor.setTextCursor(c);
        QKeyEvent bs(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier, QString());
        QApplication::sendEvent(&editor, &bs);
        TEST_ASSERT(editor.toPlainText().isEmpty(),
                    "backspace inside empty '$$' should delete both dollars");
    }

    // Comment line: '$' must NOT auto-pair inside a comment.
    {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("% "));
        editor.moveCursor(QTextCursor::End);
        sendKeyText(&editor, QStringLiteral("$"));
        TEST_ASSERT(editor.toPlainText() == QStringLiteral("% $"),
                    "'$' in a comment line should not auto-pair");
    }
}

// Feature: pressing Enter inside a "{|}" pair that sits alone on a line splits
// it over three lines with the closing brace realigned and the cursor on an
// indented middle line.
static void test_brace_enter_split()
{
    // Line-start "{|}" with leading indentation.
    {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("\\foreach \\x in {1,2,3,4,5}\n{}"));
        // Place cursor between the braces on line 2 (i.e. after '{').
        QTextCursor c = editor.textCursor();
        int idx = editor.toPlainText().indexOf(QStringLiteral("{}"));
        c.setPosition(idx + 1);
        editor.setTextCursor(c);
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\n"));
        QApplication::sendEvent(&editor, &ev);
        const QString expected =
            QStringLiteral("\\foreach \\x in {1,2,3,4,5}\n{\n    \n}");
        TEST_ASSERT(editor.toPlainText() == expected,
                    "Enter inside line-start '{|}' should split into three lines");
    }
}

// Feature: Tab / Shift+Tab (de)indent the selected line range.
static void test_tab_block_indent()
{
    // Tab indents every line in a multi-line selection by 4 spaces.
    {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("a\nb\nc"));
        QTextCursor c = editor.textCursor();
        c.setPosition(0);
        c.setPosition(3, QTextCursor::KeepAnchor); // select "a\nb"
        editor.setTextCursor(c);
        QKeyEvent tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier, QStringLiteral("\t"));
        QApplication::sendEvent(&editor, &tab);
        TEST_ASSERT(editor.toPlainText() == QStringLiteral("    a\n    b\nc"),
                    "Tab on multi-line selection should indent each line");
    }

    // Shift+Tab removes one indent level from each selected line.
    {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("        a\n    b\nc"));
        QTextCursor c = editor.textCursor();
        c.setPosition(0);
        c.setPosition(13, QTextCursor::KeepAnchor); // covers first two lines
        editor.setTextCursor(c);
        QKeyEvent bt(QEvent::KeyPress, Qt::Key_Backtab, Qt::ShiftModifier, QString());
        QApplication::sendEvent(&editor, &bt);
        TEST_ASSERT(editor.toPlainText() == QStringLiteral("    a\nb\nc"),
                    "Shift+Tab on multi-line selection should dedent each line");
    }

    // Shift+Tab with no selection dedents the current line.
    {
        CodeEditor editor;
        editor.setPlainText(QStringLiteral("        x"));
        editor.moveCursor(QTextCursor::End);
        QKeyEvent bt(QEvent::KeyPress, Qt::Key_Backtab, Qt::ShiftModifier, QString());
        QApplication::sendEvent(&editor, &bt);
        TEST_ASSERT(editor.toPlainText() == QStringLiteral("    x"),
                    "Shift+Tab without selection should dedent the current line");
    }
}

// Feature 1: the packages and TikZ-libraries metadata fields have a
// CommaListCompleter that completes only the last comma-separated segment.
static void test_metadata_field_completers(MainWindow *mw)
{
    QLineEdit *pkgEdit = mw->findChild<QLineEdit*>(QStringLiteral("metaPackagesEdit"));
    QLineEdit *libEdit = mw->findChild<QLineEdit*>(QStringLiteral("metaTikzLibrariesEdit"));
    TEST_ASSERT(pkgEdit && libEdit, "metadata fields should exist");
    if (!pkgEdit || !libEdit) return;

    auto *pkgComp = dynamic_cast<CommaListCompleter*>(pkgEdit->completer());
    auto *libComp = dynamic_cast<CommaListCompleter*>(libEdit->completer());
    TEST_ASSERT(pkgComp != nullptr, "packages field should have a CommaListCompleter");
    TEST_ASSERT(libComp != nullptr, "TikZ libraries field should have a CommaListCompleter");
    if (!pkgComp || !libComp) return;

    // splitPath keys off the last comma segment: "calc, thr" -> "thr".
    {
        const QStringList seg = libComp->splitPath(QStringLiteral("calc, thr"));
        TEST_ASSERT(seg.size() == 1 && seg.first() == QStringLiteral("thr"),
                    "libraries completer should key off the last comma segment");
    }

    // Library completer offers known libraries for a segment prefix. Check the
    // underlying source model (currentCompletion() runs pathFromIndex, which
    // rebuilds the whole field, so we inspect the raw word list instead).
    auto modelContains = [](QCompleter *comp, const QString &word) -> bool {
        QAbstractItemModel *src = comp->model();
        for (int i = 0; i < src->rowCount(); ++i)
            if (src->index(i, 0).data().toString() == word) return true;
        return false;
    };
    // With an empty field and prefix "thr", the completion model should contain
    // "through".
    libEdit->clear();
    libComp->setCompletionPrefix(QStringLiteral("thr"));
    bool foundThrough = false;
    for (int i = 0; i < libComp->completionCount(); ++i) {
        libComp->setCurrentRow(i);
        if (libComp->currentCompletion() == QStringLiteral("through")) { foundThrough = true; break; }
    }
    TEST_ASSERT(foundThrough, "library completer should offer 'through' for prefix 'thr'");
    TEST_ASSERT(modelContains(libComp, QStringLiteral("through")),
                "library completer model should include 'through'");

    // Package completer offers known packages for a segment prefix.
    pkgEdit->clear();
    pkgComp->setCompletionPrefix(QStringLiteral("amsm"));
    bool foundAms = false;
    for (int i = 0; i < pkgComp->completionCount(); ++i) {
        pkgComp->setCurrentRow(i);
        if (pkgComp->currentCompletion() == QStringLiteral("amsmath")) { foundAms = true; break; }
    }
    TEST_ASSERT(foundAms, "package completer should offer 'amsmath' for prefix 'amsm'");

    // pathFromIndex rebuilds the full field, preserving earlier entries.
    libEdit->setText(QStringLiteral("calc, thr"));
    {
        // Locate "through" in the completer's source model and rebuild via
        // pathFromIndex (which reads the live QLineEdit text set above).
        QAbstractItemModel *src = libComp->model();
        QModelIndex through;
        for (int i = 0; i < src->rowCount(); ++i) {
            QModelIndex idx = src->index(i, 0);
            if (src->data(idx).toString() == QStringLiteral("through")) { through = idx; break; }
        }
        TEST_ASSERT(through.isValid(), "'through' should exist in the library model");
        if (through.isValid()) {
            QString rebuilt = libComp->pathFromIndex(through);
            TEST_ASSERT(rebuilt == QStringLiteral("calc, through"),
                        "accepting 'through' should yield 'calc, through'");
        }
    }
}

// Tag-filter pruning: when the last snippet carrying a selected tag drops that
// tag, refreshTagFilter must (a) remove it from the tag strip, (b) deselect it,
// and (c) re-run the search so the thumbnail list is no longer filtered by a
// tag nothing has. Reproduces the "stuck selected tag → empty list, no button
// to unselect" bug.
static void test_tag_filter_prune(SnippetManager *snippetMgr, SearchPanel *searchPanel)
{
    if (!snippetMgr || !searchPanel) return;

    // A snippet that solely owns the tag "OnlyHere", plus another with a shared
    // tag so the strip is non-empty after pruning.
    QString idA = snippetMgr->createSnippet("Test TagPrune A", "test/tagprune");
    QString idB = snippetMgr->createSnippet("Test TagPrune B", "test/tagprune");
    Snippet a = snippetMgr->loadSnippet(idA);
    a.tags = QStringList{ "OnlyHere", "Shared" };
    snippetMgr->saveSnippet(a);
    Snippet b = snippetMgr->loadSnippet(idB);
    b.tags = QStringList{ "Shared" };
    snippetMgr->saveSnippet(b);

    // Rebuild the tag strip now that A/B exist so "OnlyHere" is offered.
    searchPanel->refreshTagFilter();
    QApplication::processEvents();
    TEST_ASSERT(searchPanel->allTagNames().contains("OnlyHere"),
                "tag strip should offer 'OnlyHere' while a snippet has it");

    // Select the unique tag → only snippet A should show.
    searchPanel->setTagSelected("OnlyHere", true);
    QApplication::processEvents();
    TEST_ASSERT(searchPanel->selectedTags().contains("OnlyHere"),
                "'OnlyHere' should be selected");
    TEST_ASSERT(searchPanel->thumbnailCount() == 1,
                "only snippet A should match the 'OnlyHere' filter");

    // Remove the tag from A (its only owner). This mirrors editing/saving a
    // snippet's tags in the UI, which calls refreshTagFilter().
    a = snippetMgr->loadSnippet(idA);
    a.tags = QStringList{ "Shared" };
    snippetMgr->saveSnippet(a);

    searchPanel->refreshTagFilter();
    QApplication::processEvents();

    TEST_ASSERT(!searchPanel->allTagNames().contains("OnlyHere"),
                "'OnlyHere' must disappear from the strip once unused");
    TEST_ASSERT(!searchPanel->selectedTags().contains("OnlyHere"),
                "'OnlyHere' must be auto-deselected once it no longer exists");
    TEST_ASSERT(searchPanel->thumbnailCount() >= 2,
                "list must no longer be filtered by the removed tag (both snippets show)");

    // Regression (flicker): calling refreshTagFilter() again with the SAME tag
    // set must NOT tear down and rebuild the strip — the container identity is
    // preserved, so tags don't flash expanded-then-collapsed on every save.
    const void *tokenBefore = searchPanel->tagStripToken();
    searchPanel->refreshTagFilter();
    QApplication::processEvents();
    TEST_ASSERT(searchPanel->tagStripToken() == tokenBefore,
                "refreshTagFilter with unchanged tag set must not rebuild the strip");

    snippetMgr->deleteSnippet(idA);
    snippetMgr->deleteSnippet(idB);
    searchPanel->refreshTagFilter();
    QApplication::processEvents();
}

int main(int argc, char *argv[])
{
    // Force the offscreen platform so synthetic key events are delivered to the
    // (unfocused) editor deterministically, independent of the host session
    // (Wayland/X11 only route key input to the focused window).
    // HITIKZ_TEST_PLATFORM overrides this to debug platform-specific behavior
    // (e.g. HITIKZ_TEST_PLATFORM=wayland runs on the live session).
    if (qEnvironmentVariableIsSet("HITIKZ_TEST_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", qgetenv("HITIKZ_TEST_PLATFORM"));
    else
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    QApplication app(argc, argv);
    fprintf(stderr, "=== Testing Multi-Tab Functionality ===\n");

    test_word_wrap_toggle();
    test_dollar_autopair();
    test_brace_enter_split();
    test_tab_block_indent();

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

        {
            QAction *u = findActionByText(&mw, QStringLiteral("撤销"));
            QAction *r = findActionByText(&mw, QStringLiteral("重做"));
            TEST_ASSERT(u && !u->isEnabled(), "undo disabled with zero tabs");
            TEST_ASSERT(r && !r->isEnabled(), "redo disabled with zero tabs");
        }

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
        test_metadata_field_completers(&mw);

        test_tag_filter_prune(snippetMgr, searchPanel);

        test_undo_redo_action_state(&mw, snippetMgr, searchPanel, tabWidget);

        cleanup_snippets(snippetMgr);
    }

    fprintf(stderr, "\n=== Results ===\n");
    fprintf(stderr, "Passed: %d\n", g_testsPassed);
    fprintf(stderr, "Failed: %d\n", g_testsFailed);

    return g_testsFailed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
