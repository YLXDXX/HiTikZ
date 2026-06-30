#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDebug>
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

    // Test 3: Minimal content (just a single draw command)
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
