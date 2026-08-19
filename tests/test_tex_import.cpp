#include "latex_compiler.h"
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <cstdlib>

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

    QString testDir = QDir::tempPath() + "/hitikz_tex_import_test/";
    QDir().mkpath(testDir);

    // Test 1: Extract code between \begin{document} and \end{document}
    {
        QString texContent =
            "\\documentclass[tikz]{standalone}\n"
            "\\usepackage{tikz}\n"
            "\\begin{document}\n"
            "\\begin{tikzpicture}\n"
            "\\draw (0,0) -- (1,1);\n"
            "\\end{tikzpicture}\n"
            "\\end{document}\n";

        QString fpath = testDir + "test1.tex";
        QFile f(fpath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) { g_failed++; }
        f.write(texContent.toUtf8());
        f.close();

        QFile rf(fpath);
        if (!rf.open(QIODevice::ReadOnly | QIODevice::Text)) { g_failed++; }
        QString content = QString::fromUtf8(rf.readAll());
        rf.close();

        int docBegin = content.indexOf("\\begin{document}");
        int docEnd = content.indexOf("\\end{document}");
        CHECK(docBegin >= 0 && docEnd > docBegin, "Should have document environment");

        int codeStart = content.indexOf('\n', docBegin) + 1;
        QString code = content.mid(codeStart, docEnd - codeStart).trimmed();
        CHECK(code.contains("\\begin{tikzpicture}"), "Should contain tikzpicture");
        CHECK(code.contains("\\end{tikzpicture}"), "Should contain end tikzpicture");
        CHECK(code.contains("\\draw (0,0) -- (1,1);"), "Should contain draw command");
        CHECK(!code.contains("\\documentclass"), "Should not contain documentclass");
        CHECK(!code.contains("\\usepackage"), "Should not contain usepackage");
        qDebug() << "PASS: Test 1 - Extract code from document body";
    }

    // Test 2: File with just tikzpicture (no document class)
    {
        QString texContent =
            "\\begin{tikzpicture}\n"
            "\\draw (0,0) circle (1);\n"
            "\\end{tikzpicture}";

        QString fpath = testDir + "test2.tex";
        QFile f(fpath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) { g_failed++; }
        f.write(texContent.toUtf8());
        f.close();

        QFile rf(fpath);
        if (!rf.open(QIODevice::ReadOnly | QIODevice::Text)) { g_failed++; }
        QString content = QString::fromUtf8(rf.readAll());
        rf.close();

        int docBegin = content.indexOf("\\begin{document}");
        if (docBegin >= 0) {
            qDebug() << "FAIL: Test 2a - Should have no document environment";
            g_failed++;
        } else {
            int tikzBegin = content.indexOf("\\begin{tikzpicture}");
            int tikzEnd = content.indexOf("\\end{tikzpicture}");
            CHECK(tikzBegin >= 0 && tikzEnd > tikzBegin, "Should have tikzpicture");
            QString code = content.mid(tikzBegin, tikzEnd + 17 - tikzBegin);
            CHECK(code.contains("\\begin{tikzpicture}"), "Should contain begin tikzpicture");
            CHECK(code.contains("\\end{tikzpicture}"), "Should contain end tikzpicture");
            CHECK(code.contains("\\draw (0,0) circle (1);"), "Should contain draw circle");
            qDebug() << "PASS: Test 2 - Extract tikzpicture from standalone file";
        }
    }

    // Test 3: Fallback to full content when no tikzpicture or document
    {
        QString texContent = "\\draw (0,0) -- (2,2);";

        QString fpath = testDir + "test3.tex";
        QFile f(fpath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) { g_failed++; }
        f.write(texContent.toUtf8());
        f.close();

        QFile rf(fpath);
        if (!rf.open(QIODevice::ReadOnly | QIODevice::Text)) { g_failed++; }
        QString content = QString::fromUtf8(rf.readAll());
        rf.close();

        int docBegin = content.indexOf("\\begin{document}");
        int tikzBegin = content.indexOf("\\begin{tikzpicture}");
        QString code;
        if (docBegin >= 0) {
            qDebug() << "FAIL: Test 3a - Should have no document";
            g_failed++;
        } else if (tikzBegin >= 0) {
            qDebug() << "FAIL: Test 3b - Should have no tikzpicture";
            g_failed++;
        } else {
            code = content.trimmed();
            CHECK(code == "\\draw (0,0) -- (2,2);", "Fallback code should match");
            qDebug() << "PASS: Test 3 - Fallback to full content";
        }
    }

    // Test 4: Parse \usepackage from preamble
    {
        QString preamble =
            "\\documentclass[tikz]{standalone}\n"
            "\\usepackage{tikz}\n"
            "\\usepackage{amsmath, amssymb}\n"
            "\\usepackage[tikz]{standalone}\n"
            "\\usepackage[european, nosiunitx]{circuitikz}\n";

        QStringList packages;
        QRegularExpression usepkgRe("\\\\usepackage(?:\\[([^\\]]*)\\])?\\{([^}]*)\\}");
        QRegularExpressionMatchIterator it = usepkgRe.globalMatch(preamble);

        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            QString options = m.captured(1);
            QString pkgList = m.captured(2);
            QStringList pkgs = pkgList.split(',');
            for (const QString &pkg : pkgs) {
                QString trimmed = pkg.trimmed();
                if (trimmed.isEmpty()) continue;
                if (!options.isEmpty())
                    trimmed = "[" + options + "]" + trimmed;
                packages.append(trimmed);
            }
        }

        CHECK(packages.size() >= 5, "Should have at least 5 packages");
        CHECK(packages.contains("tikz"), "Should contain 'tikz'");
        CHECK(packages.contains("amsmath"), "Should contain 'amsmath'");
        CHECK(packages.contains("[european, nosiunitx]circuitikz"), "Should contain circuitikz with options");
        qDebug() << "PASS: Test 4 - Package parsing from preamble";
    }

    // Test 5: Parse \usetikzlibrary from preamble
    {
        QString preamble =
            "\\usetikzlibrary{calc, arrows, shapes}\n"
            "\\usetikzlibrary{patterns, decorations.pathmorphing}\n";

        QStringList libraries;
        QRegularExpression uselibRe("\\\\usetikzlibrary\\{([^}]*)\\}");
        QRegularExpressionMatchIterator it = uselibRe.globalMatch(preamble);

        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            QString libList = m.captured(1);
            QStringList libs = libList.split(',');
            for (const QString &lib : libs) {
                QString trimmed = lib.trimmed();
                if (!trimmed.isEmpty())
                    libraries.append(trimmed);
            }
        }

        CHECK(libraries.size() >= 5, "Should have at least 5 libraries");
        CHECK(libraries.contains("calc"), "Should contain 'calc'");
        CHECK(libraries.contains("decorations.pathmorphing"), "Should contain 'decorations.pathmorphing'");
        qDebug() << "PASS: Test 5 - TikZ library parsing from preamble";
    }

    // Test 6: Full .tex file with packages and libraries extraction
    {
        QString texContent =
            "\\documentclass[tikz, border=5pt]{standalone}\n"
            "\\usepackage{tikz}\n"
            "\\usepackage{xcolor}\n"
            "\\usepackage{tikz-3dplot}\n"
            "\\usepackage[european]{circuitikz}\n"
            "\\usetikzlibrary{calc, angles, quotes}\n"
            "\\begin{document}\n"
            "\\begin{tikzpicture}\n"
            "\\draw (0,0) -- (1,1) node[right] {Hello};\n"
            "\\end{tikzpicture}\n"
            "\\end{document}\n";

        QString fpath = testDir + "test6.tex";
        QFile f(fpath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) { g_failed++; }
        f.write(texContent.toUtf8());
        f.close();

        QFile rf(fpath);
        if (!rf.open(QIODevice::ReadOnly | QIODevice::Text)) { g_failed++; }
        QString content = QString::fromUtf8(rf.readAll());
        rf.close();

        int docBegin = content.indexOf("\\begin{document}");
        int docEnd = content.indexOf("\\end{document}");
        CHECK(docBegin >= 0 && docEnd > docBegin, "Should have document environment");

        QString preamble = content.left(docBegin);
        int codeStart = content.indexOf('\n', docBegin) + 1;
        QString code = content.mid(codeStart, docEnd - codeStart).trimmed();

        CHECK(code.contains("\\begin{tikzpicture}"), "tikzpicture should be in extracted code");

        QStringList packages;
        QRegularExpression usepkgRe("\\\\usepackage(?:\\[([^\\]]*)\\])?\\{([^}]*)\\}");
        QRegularExpressionMatchIterator pkgIt = usepkgRe.globalMatch(preamble);
        while (pkgIt.hasNext()) {
            QRegularExpressionMatch m = pkgIt.next();
            QString options = m.captured(1);
            QString pkgList = m.captured(2);
            QStringList pkgs = pkgList.split(',');
            for (const QString &pkg : pkgs) {
                QString trimmed = pkg.trimmed();
                if (trimmed.isEmpty()) continue;
                if (!options.isEmpty())
                    trimmed = "[" + options + "]" + trimmed;
                packages.append(trimmed);
            }
        }

        CHECK(packages.contains("tikz-3dplot"), "tikz-3dplot should be parsed");
        CHECK(packages.contains("[european]circuitikz"), "circuitikz with option should be parsed");

        QStringList libraries;
        QRegularExpression uselibRe("\\\\usetikzlibrary\\{([^}]*)\\}");
        QRegularExpressionMatchIterator libIt = uselibRe.globalMatch(preamble);
        while (libIt.hasNext()) {
            QRegularExpressionMatch m = libIt.next();
            QString libList = m.captured(1);
            QStringList libs = libList.split(',');
            for (const QString &lib : libs) {
                QString trimmed = lib.trimmed();
                if (!trimmed.isEmpty())
                    libraries.append(trimmed);
            }
        }

        CHECK(libraries.contains("calc"), "calc library should be parsed");
        CHECK(libraries.contains("angles"), "angles library should be parsed");
        CHECK(libraries.contains("quotes"), "quotes library should be parsed");

        qDebug() << "PASS: Test 6 - Full .tex file with packages and libraries";

        QFile::remove(fpath);
    }

    // Test 7: metadataHeader builds the leading comment block
    {
        const QString header = LatexCompiler::metadataHeader(
            QStringLiteral("空间几何作图"), QStringLiteral("几何简介"), QStringLiteral("几何, 空间"));
        CHECK(header == QStringLiteral("%% name: 空间几何作图\n"
                                       "%% description: 几何简介\n"
                                       "%% tags: 几何, 空间\n"),
              "header carries all three fields in order");

        // Empty fields emit no line.
        const QString noName = LatexCompiler::metadataHeader(
            QString(), QStringLiteral("只有简介"), QString());
        CHECK(!noName.contains(QStringLiteral("%% name:")), "empty name emits no name line");
        CHECK(!noName.contains(QStringLiteral("%% tags:")), "empty tags emit no tags line");
        CHECK(noName.contains(QStringLiteral("%% description: 只有简介")),
              "description line still emitted");

        CHECK(LatexCompiler::metadataHeader(QString(), QString(), QString()).isEmpty(),
              "all-empty metadata yields an empty header");

        // Multi-line descriptions become consecutive comment lines.
        const QString multi = LatexCompiler::metadataHeader(
            QStringLiteral("n"), QStringLiteral("第一行\n\n第三行"), QString());
        CHECK(multi == QStringLiteral("%% name: n\n"
                                      "%% description: 第一行\n"
                                      "%% description: \n"
                                      "%% description: 第三行\n"),
              "multi-line description emits one comment line per line");
        qDebug() << "PASS: Test 7 - metadataHeader generation";
    }

    // Test 8: extractMetadataHeader parses and strips the block
    {
        const QString doc = QStringLiteral(
            "%% name: 空间几何作图\n"
            "%% description: 第一行\n"
            "%% description: 第二行\n"
            "%% tags: 几何, 空间\n"
            "\\documentclass[tikz]{standalone}\n"
            "\\begin{document}\n"
            "\\begin{tikzpicture}\\end{tikzpicture}\n"
            "\\end{document}\n");
        QString name, desc, tags;
        const QString rest = LatexCompiler::extractMetadataHeader(doc, name, desc, tags);
        CHECK(name == QStringLiteral("空间几何作图"), "name parsed");
        CHECK(desc == QStringLiteral("第一行\n第二行"), "multi-line description joined");
        CHECK(tags == QStringLiteral("几何, 空间"), "tags parsed");
        CHECK(!rest.contains(QStringLiteral("%% name:")), "metadata lines stripped");
        CHECK(!rest.contains(QStringLiteral("%% description:")), "description lines stripped");
        CHECK(rest.startsWith(QStringLiteral("\\documentclass")), "rest starts at documentclass");
        qDebug() << "PASS: Test 8 - extractMetadataHeader parsing and stripping";
    }

    // Test 9: header round trip
    {
        const QString name = QStringLiteral("名字");
        const QString desc = QStringLiteral("第一行\n第二行\n\n第四行");
        const QString tags = QStringLiteral("标签A, 标签B");
        const QString doc = LatexCompiler::metadataHeader(name, desc, tags)
            + QStringLiteral("\\documentclass{standalone}\n");

        QString n2, d2, t2;
        const QString rest = LatexCompiler::extractMetadataHeader(doc, n2, d2, t2);
        CHECK(n2 == name, "round-trip name");
        CHECK(d2 == desc, "round-trip multi-line description");
        CHECK(t2 == tags, "round-trip tags");
        CHECK(rest == QStringLiteral("\\documentclass{standalone}\n"),
              "round-trip keeps the document intact");
        qDebug() << "PASS: Test 9 - metadata round trip";
    }

    // Test 10: no metadata — content unchanged, out-params untouched
    {
        const QString doc = QStringLiteral(
            "% 普通注释\n"
            "\\documentclass[tikz]{standalone}\n"
            "\\begin{document}\n"
            "\\begin{tikzpicture}\\end{tikzpicture}\n"
            "\\end{document}\n");
        QString name(QStringLiteral("pre")), desc, tags;
        const QString rest = LatexCompiler::extractMetadataHeader(doc, name, desc, tags);
        CHECK(name == QStringLiteral("pre"), "absent name keeps previous value");
        CHECK(desc.isEmpty(), "description empty when absent");
        CHECK(tags.isEmpty(), "tags empty when absent");
        CHECK(rest == doc, "content unchanged without metadata");
        qDebug() << "PASS: Test 10 - documents without metadata are untouched";
    }

    // Test 11: lookalike comments inside the body are not parsed
    {
        const QString doc = QStringLiteral(
            "\\documentclass[tikz]{standalone}\n"
            "\\begin{document}\n"
            "%% name: 不要解析\n"
            "\\begin{tikzpicture}\\end{tikzpicture}\n"
            "\\end{document}\n");
        QString name, desc, tags;
        LatexCompiler::extractMetadataHeader(doc, name, desc, tags);
        CHECK(name.isEmpty(), "body comment is not metadata");
        CHECK(desc.isEmpty(), "body description comment ignored");
        qDebug() << "PASS: Test 11 - lookalike comments in the body are ignored";
    }

    // Test 12: metadata in a tikzpicture-only file (leading block)
    {
        const QString doc = QStringLiteral(
            "%% name: 只有图\n"
            "%% tags: 单图\n"
            "\\begin{tikzpicture}\n"
            "\\draw (0,0) -- (1,1);\n"
            "\\end{tikzpicture}\n");
        QString name, desc, tags;
        const QString rest = LatexCompiler::extractMetadataHeader(doc, name, desc, tags);
        CHECK(name == QStringLiteral("只有图"), "name parsed before tikzpicture");
        CHECK(tags == QStringLiteral("单图"), "tags parsed before tikzpicture");
        CHECK(rest.startsWith(QStringLiteral("\\begin{tikzpicture}")),
              "rest starts at tikzpicture");
        qDebug() << "PASS: Test 12 - metadata in tikzpicture-only files";
    }

    // Test 13: single '%' accepted, first occurrence wins
    {
        const QString doc = QStringLiteral(
            "% name: 甲\n"
            "%% name: 乙\n"
            "% tags: t1\n"
            "%% tags: t2\n"
            "\\documentclass{standalone}\n");
        QString name, desc, tags;
        LatexCompiler::extractMetadataHeader(doc, name, desc, tags);
        CHECK(name == QStringLiteral("甲"), "first name line wins");
        CHECK(tags == QStringLiteral("t1"), "first tags line wins");
        qDebug() << "PASS: Test 13 - single % and first-occurrence semantics";
    }

    QDir(testDir).removeRecursively();

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) failed!";
        return 1;
    }

    qDebug() << "\nAll .tex import tests passed!";
    return 0;
}
