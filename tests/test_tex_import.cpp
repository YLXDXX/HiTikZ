#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <cassert>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    int failed = 0;
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
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        f.write(texContent.toUtf8());
        f.close();

        // Read and parse
        QFile rf(fpath);
        rf.open(QIODevice::ReadOnly | QIODevice::Text);
        QString content = QString::fromUtf8(rf.readAll());
        rf.close();

        int docBegin = content.indexOf("\\begin{document}");
        int docEnd = content.indexOf("\\end{document}");
        assert(docBegin >= 0 && docEnd > docBegin);

        int codeStart = content.indexOf('\n', docBegin) + 1;
        QString code = content.mid(codeStart, docEnd - codeStart).trimmed();
        assert(code.contains("\\begin{tikzpicture}"));
        assert(code.contains("\\end{tikzpicture}"));
        assert(code.contains("\\draw (0,0) -- (1,1);"));
        assert(!code.contains("\\documentclass"));
        assert(!code.contains("\\usepackage"));
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
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        f.write(texContent.toUtf8());
        f.close();

        QFile rf(fpath);
        rf.open(QIODevice::ReadOnly | QIODevice::Text);
        QString content = QString::fromUtf8(rf.readAll());
        rf.close();

        int docBegin = content.indexOf("\\begin{document}");
        if (docBegin >= 0) {
            qDebug() << "FAIL: Test 2a - Should have no document environment";
            failed++;
        } else {
            int tikzBegin = content.indexOf("\\begin{tikzpicture}");
            int tikzEnd = content.indexOf("\\end{tikzpicture}");
            assert(tikzBegin >= 0 && tikzEnd > tikzBegin);
            QString code = content.mid(tikzBegin, tikzEnd + 17 - tikzBegin);
            assert(code.contains("\\begin{tikzpicture}"));
            assert(code.contains("\\end{tikzpicture}"));
            assert(code.contains("\\draw (0,0) circle (1);"));
            qDebug() << "PASS: Test 2 - Extract tikzpicture from standalone file";
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

        if (packages.size() < 5) {
            qDebug() << "FAIL: Test 4a - Expected at least 5 packages, got" << packages.size();
            failed++;
        } else if (!packages.contains("tikz")) {
            qDebug() << "FAIL: Test 4b - 'tikz' package not found";
            failed++;
        } else if (!packages.contains("amsmath")) {
            qDebug() << "FAIL: Test 4c - 'amsmath' package not found";
            failed++;
        } else if (!packages.contains("[european, nosiunitx]circuitikz")) {
            qDebug() << "FAIL: Test 4d - circuitikz with options not parsed correctly";
            failed++;
        } else {
            qDebug() << "PASS: Test 4 - Package parsing from preamble";
        }
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

        if (libraries.size() < 5) {
            qDebug() << "FAIL: Test 5a - Expected at least 5 libraries, got" << libraries.size();
            failed++;
        } else if (!libraries.contains("calc")) {
            qDebug() << "FAIL: Test 5b - 'calc' library not found";
            failed++;
        } else if (!libraries.contains("decorations.pathmorphing")) {
            qDebug() << "FAIL: Test 5c - 'decorations.pathmorphing' not found";
            failed++;
        } else {
            qDebug() << "PASS: Test 5 - TikZ library parsing from preamble";
        }
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
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        f.write(texContent.toUtf8());
        f.close();

        QFile rf(fpath);
        rf.open(QIODevice::ReadOnly | QIODevice::Text);
        QString content = QString::fromUtf8(rf.readAll());
        rf.close();

        int docBegin = content.indexOf("\\begin{document}");
        int docEnd = content.indexOf("\\end{document}");
        assert(docBegin >= 0 && docEnd > docBegin);

        QString preamble = content.left(docBegin);
        int codeStart = content.indexOf('\n', docBegin) + 1;
        QString code = content.mid(codeStart, docEnd - codeStart).trimmed();

        // Verify code extraction
        if (!code.contains("\\begin{tikzpicture}")) {
            qDebug() << "FAIL: Test 6a - tikzpicture not in extracted code";
            failed++;
        }

        // Verify package parsing
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

        if (!packages.contains("tikz-3dplot")) {
            qDebug() << "FAIL: Test 6b - tikz-3dplot not parsed";
            failed++;
        } else if (!packages.contains("[european]circuitikz")) {
            qDebug() << "FAIL: Test 6c - [european]circuitikz not parsed";
            failed++;
        }

        // Verify library parsing
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

        if (!libraries.contains("calc")) {
            qDebug() << "FAIL: Test 6d - calc library not parsed";
            failed++;
        } else if (!libraries.contains("angles")) {
            qDebug() << "FAIL: Test 6e - angles library not parsed";
            failed++;
        } else if (!libraries.contains("quotes")) {
            qDebug() << "FAIL: Test 6f - quotes library not parsed";
            failed++;
        }

        if (failed == 0) {  // no failures from any test yet
            qDebug() << "PASS: Test 6 - Full .tex file with packages and libraries";
        }

        QFile::remove(fpath);
    }
    {
        QString texContent = "\\draw (0,0) -- (2,2);";

        QString fpath = testDir + "test3.tex";
        QFile f(fpath);
        f.open(QIODevice::WriteOnly | QIODevice::Text);
        f.write(texContent.toUtf8());
        f.close();

        QFile rf(fpath);
        rf.open(QIODevice::ReadOnly | QIODevice::Text);
        QString content = QString::fromUtf8(rf.readAll());
        rf.close();

        int docBegin = content.indexOf("\\begin{document}");
        int tikzBegin = content.indexOf("\\begin{tikzpicture}");
        QString code;
        if (docBegin >= 0) {
            qDebug() << "FAIL: Test 3a - Should have no document";
            failed++;
        } else if (tikzBegin >= 0) {
            qDebug() << "FAIL: Test 3b - Should have no tikzpicture";
            failed++;
        } else {
            code = content.trimmed();
            assert(code == "\\draw (0,0) -- (2,2);");
            qDebug() << "PASS: Test 3 - Fallback to full content";
        }
    }

    // Cleanup
    QDir(testDir).removeRecursively();

    if (failed > 0) {
        qDebug() << "\n" << failed << "test(s) failed!";
        return 1;
    }

    qDebug() << "\nAll .tex import tests passed!";
    return 0;
}
