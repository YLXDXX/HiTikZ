#include <QApplication>
#include <QTabWidget>
#include <QTabBar>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "../src/mainwindow.h"
#include "../src/snippet_manager.h"
#include "../src/search_panel.h"

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

static void cleanup_snippets(SnippetManager *mgr)
{
    QList<Snippet> all = mgr->getAllSnippets(true);
    for (const Snippet &s : all) {
        if (s.category.startsWith("test/") || s.name.startsWith("Test ")) {
            mgr->deleteSnippet(s.id);
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    QApplication app(argc, argv);

    fprintf(stderr, "=== Testing Multi-Tab Functionality ===\n");

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

        cleanup_snippets(snippetMgr);
    }

    fprintf(stderr, "\n=== Results ===\n");
    fprintf(stderr, "Passed: %d\n", g_testsPassed);
    fprintf(stderr, "Failed: %d\n", g_testsFailed);

    return g_testsFailed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
