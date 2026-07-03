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

    QDir(testDir).removeRecursively();

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) failed!";
        return 1;
    }

    qDebug() << "\nAll .tex import tests passed!";
    return 0;
}
