#include "snippet_manager.h"
#include <QCoreApplication>
#include <QDebug>
#include <cassert>
#include <cstdio>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    int failed = 0;

    // Test 1: Exact match
    {
        int score = SnippetManager::fuzzyMatchScore("circle", "circle");
        fprintf(stderr, "%s: Test 1 - Exact match score=%d (expected >0)\n",
            score > 0 ? "PASS" : "FAIL", score);
        if (score <= 0) failed++;
    }

    // Test 2: Subsequence match
    {
        int score = SnippetManager::fuzzyMatchScore("crc", "circle");
        fprintf(stderr, "%s: Test 2 - Subsequence 'crc' in 'circle' score=%d (expected >0)\n",
            score > 0 ? "PASS" : "FAIL", score);
        if (score <= 0) failed++;
    }

    // Test 3: Consecutive bonus
    {
        int scoreConsecutive = SnippetManager::fuzzyMatchScore("ci", "circle");
        int scoreNonCon = SnippetManager::fuzzyMatchScore("ce", "circle");
        fprintf(stderr, "%s: Test 3 - Consecutive 'ci' (%d) > non-consecutive 'ce' (%d)\n",
            scoreConsecutive > scoreNonCon ? "PASS" : "FAIL",
            scoreConsecutive, scoreNonCon);
        if (scoreConsecutive <= scoreNonCon) failed++;
    }

    // Test 4: No match
    {
        int score = SnippetManager::fuzzyMatchScore("xyz", "circle");
        fprintf(stderr, "%s: Test 4 - No match 'xyz' in 'circle' score=%d (expected 0)\n",
            score == 0 ? "PASS" : "FAIL", score);
        if (score != 0) failed++;
    }

    // Test 5: Empty query matches all
    {
        int score = SnippetManager::fuzzyMatchScore("", "anything");
        fprintf(stderr, "%s: Test 5 - Empty query score=%d (expected 100)\n",
            score == 100 ? "PASS" : "FAIL", score);
        if (score != 100) failed++;
    }

    // Test 6: Case insensitivity
    {
        int scoreLower = SnippetManager::fuzzyMatchScore("circle", "Circle");
        int scoreUpper = SnippetManager::fuzzyMatchScore("CIRCLE", "circle");
        fprintf(stderr, "%s: Test 6 - Case insensitive match (lower=%d, upper=%d)\n",
            (scoreLower > 0 && scoreUpper > 0) ? "PASS" : "FAIL",
            scoreLower, scoreUpper);
        if (scoreLower <= 0 || scoreUpper <= 0) failed++;
    }

    // Test 7: searchSnippets with real data
    {
        SnippetManager mgr;
        QString id1 = mgr.createSnippet("圆的画法", "math/geometry");
        QString id2 = mgr.createSnippet("矩形边框", "math/geometry");
        QString id3 = mgr.createSnippet("物理电路图", "physics/circuits");

        Snippet s1 = mgr.loadSnippet(id1);
        s1.description = "如何画一个圆";
        mgr.saveSnippet(s1);

        Snippet s2 = mgr.loadSnippet(id2);
        s2.description = "用圆圆润润的方式画矩形";
        mgr.saveSnippet(s2);

        Snippet s3 = mgr.loadSnippet(id3);
        s3.description = "电路图中的圆形符号";
        mgr.saveSnippet(s3);

        QList<SearchResult> results = mgr.searchSnippets("圆");

        fprintf(stderr, "%s: Test 7a - Search '圆' returns %d results (expected >=3)\n",
            results.size() >= 3 ? "PASS" : "FAIL", results.size());
        if (results.size() < 3) failed++;

        if (!results.isEmpty()) {
            fprintf(stderr, "  Top result: '%s' score=%d\n",
                qPrintable(results.first().snippet.name), results.first().score);
        }

        QList<SearchResult> emptyResults = mgr.searchSnippets("xyz_not_found");
        fprintf(stderr, "%s: Test 7b - Search 'xyz_not_found' returns %d results (expected 0)\n",
            emptyResults.isEmpty() ? "PASS" : "FAIL", emptyResults.size());
        if (!emptyResults.isEmpty()) failed++;

        mgr.deleteSnippet(id1);
        mgr.deleteSnippet(id2);
        mgr.deleteSnippet(id3);
    }

    // Test 8: getAllCategories
    {
        SnippetManager mgr;
        QString id1 = mgr.createSnippet("测试A", "cat/alpha");
        QString id2 = mgr.createSnippet("测试B", "cat/beta");
        QString id3 = mgr.createSnippet("测试C", "cat/alpha");

        QStringList cats = mgr.getAllCategories();
        fprintf(stderr, "%s: Test 8 - getAllCategories returns %d unique cats (expected 2)\n",
            cats.size() == 2 ? "PASS" : "FAIL", cats.size());
        if (cats.size() != 2) failed++;

        mgr.deleteSnippet(id1);
        mgr.deleteSnippet(id2);
        mgr.deleteSnippet(id3);
    }

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }
    fprintf(stderr, "\nAll search tests passed!\n");
    return 0;
}
