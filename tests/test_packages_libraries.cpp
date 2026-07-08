#include "snippet_manager.h"
#include "latex_compiler.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <cassert>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    int failed = 0;

    SnippetManager mgr;

    // --- Test 1: Snippet JSON serialization with packages and tikzLibraries ---
    {
        QString id = mgr.createSnippet("PackagesTest", "test/packages");
        assert(!id.isEmpty());

        Snippet s = mgr.loadSnippet(id);
        s.packages = "tikz-3dplot,[european,nosiunitx]circuitikz,tikz-cd";
        s.tikzLibraries = "calc,er,angles";
        assert(mgr.saveSnippet(s));

        Snippet reloaded = mgr.loadSnippet(id);
        assert(reloaded.packages == s.packages);
        assert(reloaded.tikzLibraries == s.tikzLibraries);
        qDebug() << "PASS: Test 1 - packages and tikzLibraries survive save/load roundtrip";

        mgr.deleteSnippet(id);
    }

    // --- Test 2: wrapCode injects \usepackage lines correctly ---
    {
        LatexCompiler compiler;
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            QString(),
            "tikz-3dplot,[european,nosiunitx]circuitikz,tikz-cd",
            QString()
        );

        assert(result.contains("\\usepackage{tikz-3dplot}"));
        assert(result.contains("\\usepackage[european,nosiunitx]{circuitikz}"));
        assert(result.contains("\\usepackage{tikz-cd}"));
        assert(result.indexOf("\\usepackage{tikz-3dplot}") < result.indexOf("\\begin{document}"));
        qDebug() << "PASS: Test 2 - packages parsed into correct \\usepackage lines before \\begin{document}";
    }

    // --- Test 3: wrapCode injects \usetikzlibrary correctly ---
    {
        LatexCompiler compiler;
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            QString(),
            QString(),
            "calc,er,angles"
        );

        assert(result.contains("\\usetikzlibrary{calc,er,angles}"));
        assert(result.indexOf("\\usetikzlibrary{calc,er,angles}") < result.indexOf("\\begin{document}"));
        qDebug() << "PASS: Test 3 - tikzLibraries parsed into correct \\usetikzlibrary line";
    }

    // --- Test 4: Both packages and tikzLibraries together ---
    {
        LatexCompiler compiler;
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            QString(),
            "tikz-3dplot",
            "calc,er,angles"
        );

        assert(result.contains("\\usepackage{tikz-3dplot}"));
        assert(result.contains("\\usetikzlibrary{calc,er,angles}"));
        int usepkgPos = result.indexOf("\\usepackage{tikz-3dplot}");
        int uselibPos = result.indexOf("\\usetikzlibrary{calc,er,angles}");
        int docBeginPos = result.indexOf("\\begin{document}");
        assert(usepkgPos < docBeginPos);
        assert(uselibPos < docBeginPos);
        qDebug() << "PASS: Test 4 - both packages and tikzLibraries injected before \\begin{document}";
    }

    // --- Test 5: Empty strings don't break anything ---
    {
        LatexCompiler compiler;
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            QString(),
            QString(),
            QString()
        );

        assert(result.contains("\\begin{tikzpicture}"));
        assert(result.contains("\\end{tikzpicture}"));
        assert(result.contains("\\begin{document}"));
        assert(result.contains("\\end{document}"));
        qDebug() << "PASS: Test 5 - empty packages and tikzLibraries produce valid output";
    }

    // --- Test 6: Snippet with packages and tikzLibraries in meta.json ---
    {
        QString id = mgr.createSnippet("MetaTest", "test/meta");
        Snippet s = mgr.loadSnippet(id);
        s.packages = "amsmath,amssymb";
        s.tikzLibraries = "arrows,shapes,patterns";
        mgr.saveSnippet(s);

        QString path = mgr.getBasePath() + id + "/meta.json";
        QFile metaFile(path);
        assert(metaFile.open(QIODevice::ReadOnly));
        QByteArray data = metaFile.readAll();
        metaFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        assert(doc.isObject());
        QJsonObject obj = doc.object();
        assert(obj.value("packages").toString() == "amsmath,amssymb");
        assert(obj.value("tikzLibraries").toString() == "arrows,shapes,patterns");
        qDebug() << "PASS: Test 6 - meta.json contains packages and tikzLibraries fields";

        mgr.deleteSnippet(id);
    }

    // --- Test 7: Template-based wrapCode with extra packages ---
    {
        LatexCompiler compiler;
        QString tmplDir = QDir::tempPath() + "/test_templates_7";
        QDir().mkpath(tmplDir);

        QFile tmplFile(tmplDir + "/test_tpl.tex");
        bool opened = tmplFile.open(QIODevice::WriteOnly | QIODevice::Text);
        Q_UNUSED(opened);
        tmplFile.write(
            "\\documentclass[tikz, border=5pt]{standalone}\n"
            "\\usepackage{tikz}\n"
            "\\usepackage{xcolor}\n"
            "\\begin{document}\n"
            "%%% TIKZ_CODE_HERE %%%\n"
            "\\end{document}\n"
        );
        tmplFile.close();

        compiler.setTemplateDir(tmplDir);
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            "test_tpl",
            "tikz-3dplot",
            "calc,angles"
        );

        assert(result.contains("\\usepackage{tikz-3dplot}"));
        assert(result.contains("\\usetikzlibrary{calc,angles}"));
        assert(!result.contains("%%% TIKZ_CODE_HERE %%%"));
        qDebug() << "PASS: Test 7 - template-based wrapCode with extra packages and libraries";

        tmplFile.remove();
        QDir().rmpath(tmplDir);
    }

    // --- Test 8: Package with options only (e.g. [european]circuitikz) ---
    {
        LatexCompiler compiler;
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            QString(),
            "[european]circuitikz",
            QString()
        );

        assert(result.contains("\\usepackage[european]{circuitikz}"));
        qDebug() << "PASS: Test 8 - single optional package parameter parsed correctly";
    }

    // --- Test 9: Complex packages with mixed optional and non-optional ---
    {
        LatexCompiler compiler;
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            QString(),
            "pgfplots,[american,nooldvoltagedirection]circuitikz,amsmath",
            QString()
        );

        assert(result.contains("\\usepackage{pgfplots}"));
        assert(result.contains("\\usepackage[american,nooldvoltagedirection]{circuitikz}"));
        assert(result.contains("\\usepackage{amsmath}"));
        qDebug() << "PASS: Test 9 - mixed optional and non-optional packages";
    }

    // --- Test 10: Malformed package (missing ']') is not silently dropped ---
    {
        LatexCompiler compiler;
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            QString(),
            "[european circuitikz",   // missing closing ']'
            QString()
        );

        // The package must still appear (as a plain package) rather than vanish.
        assert(result.contains("\\usepackage{european circuitikz}"));
        assert(result.indexOf("\\usepackage{european circuitikz}")
               < result.indexOf("\\begin{document}"));
        qDebug() << "PASS: Test 10 - malformed package (missing ']') falls back to plain package";
    }

    // --- Test 11: A malformed package after a valid one keeps the valid one ---
    {
        LatexCompiler compiler;
        QString result = compiler.wrapCode(
            "\\begin{tikzpicture}\\draw(0,0)--(1,1);\\end{tikzpicture}",
            QString(),
            "amsmath,[european circuitikz",  // valid, then malformed (missing ']')
            QString()
        );

        assert(result.contains("\\usepackage{amsmath}"));
        assert(result.contains("\\usepackage{european circuitikz}"));
        qDebug() << "PASS: Test 11 - valid package kept alongside a trailing malformed one";
    }

    if (failed > 0) {
        qDebug() << "\n" << failed << "test(s) failed!";
        return 1;
    }

    qDebug() << "\nAll packages & tikzLibraries tests passed!";
    return 0;
}
