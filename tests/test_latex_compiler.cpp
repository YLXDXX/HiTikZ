#include "latex_compiler.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <cassert>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    int failed = 0;

    // Test 1: Check xelatex availability
    {
        bool avail = LatexCompiler::checkXelatexAvailable();
        if (!avail) { qDebug() << "FAIL: Test 1 - xelatex not available"; failed++; }
        else qDebug() << "PASS: Test 1 - xelatex available";
    }

    // Test 2: Check pdftocairo availability
    {
        bool avail = LatexCompiler::checkPdfToCairoAvailable();
        if (!avail) { qDebug() << "FAIL: Test 2 - pdftocairo not available"; failed++; }
        else qDebug() << "PASS: Test 2 - pdftocairo available";
    }

    // Test 3: Basic compilation
    {
        LatexCompiler compiler;
        compiler.setXelatexPath("xelatex");

        bool finished = false;
        bool success = false;
        QString logOutput;

        QObject::connect(&compiler, &LatexCompiler::compilationFinished,
            [&](bool ok, const QString &, const QString &log) {
                finished = true;
                success = ok;
                logOutput = log;
            });

        compiler.compile("\\begin{tikzpicture}\n\\draw (0,0) -- (1,1) node[right] {Hello};\n\\end{tikzpicture}", "", "test1");

        QEventLoop loop;
        QTimer::singleShot(15000, &loop, &QEventLoop::quit);
        QObject::connect(&compiler, &LatexCompiler::compilationFinished, &loop, &QEventLoop::quit);
        loop.exec();

        if (!finished) { qDebug() << "FAIL: Test 3 - compilation timed out"; failed++; }
        else if (!success) { qDebug() << "FAIL: Test 3 - compilation failed:" << logOutput.mid(0, 1000); failed++; }
        else qDebug() << "PASS: Test 3 - basic compilation succeeded";
    }

    // Test 4: Verify PDF and PNG conversion
    {
        LatexCompiler compiler;
        compiler.setXelatexPath("xelatex");

        bool success = false;
        QString pdfPath;

        QObject::connect(&compiler, &LatexCompiler::compilationFinished,
            [&](bool ok, const QString &pdf, const QString &) {
                success = ok;
                pdfPath = pdf;
            });

        compiler.compile("\\begin{tikzpicture}\n\\draw (0,0) circle (1);\n\\end{tikzpicture}", "", "test2");

        QEventLoop loop;
        QTimer::singleShot(15000, &loop, &QEventLoop::quit);
        QObject::connect(&compiler, &LatexCompiler::compilationFinished, &loop, &QEventLoop::quit);
        loop.exec();

        if (!success || !QFile::exists(pdfPath)) {
            qDebug() << "FAIL: Test 4a - PDF not generated"; failed++;
        } else {
            qDebug() << "PASS: Test 4a - PDF file exists";

            bool convFinished = false;
            bool convSuccess = false;
            QString pngPath;

            QObject::connect(&compiler, &LatexCompiler::conversionFinished,
                [&](bool ok, const QString &path) {
                    convFinished = true;
                    convSuccess = ok;
                    pngPath = path;
                });

            compiler.convertToPng(150);

            QEventLoop loop2;
            QTimer::singleShot(10000, &loop2, &QEventLoop::quit);
            QObject::connect(&compiler, &LatexCompiler::conversionFinished, &loop2, &QEventLoop::quit);
            loop2.exec();

            if (!convFinished || !convSuccess) {
                qDebug() << "FAIL: Test 4b - PNG conversion failed"; failed++;
            } else if (!QFile::exists(pngPath)) {
                qDebug() << "FAIL: Test 4c - PNG file does not exist"; failed++;
            } else {
                qDebug() << "PASS: Test 4b - PNG conversion and file exists";
            }
        }

        // SVG conversion sub-test
        if (success && QFile::exists(pdfPath)) {
            LatexCompiler compiler2;
            bool svgConvFinished = false;
            bool svgConvSuccess = false;
            QString svgPath;

            QObject::connect(&compiler2, &LatexCompiler::conversionFinished,
                [&](bool ok, const QString &path) {
                    svgConvFinished = true;
                    svgConvSuccess = ok;
                    svgPath = path;
                });

            compiler2.convertToSvg(pdfPath);

            QEventLoop loop3;
            QTimer::singleShot(10000, &loop3, &QEventLoop::quit);
            QObject::connect(&compiler2, &LatexCompiler::conversionFinished, &loop3, &QEventLoop::quit);
            loop3.exec();

            if (!svgConvFinished || !svgConvSuccess) {
                qDebug() << "FAIL: Test 4c - SVG conversion failed"; failed++;
            } else if (!QFile::exists(svgPath)) {
                qDebug() << "FAIL: Test 4d - SVG file does not exist"; failed++;
            } else {
                qDebug() << "PASS: Test 4c - SVG conversion and file exists";
            }
        }
    }

    // Test 5: Compilation with error
    {
        LatexCompiler compiler;
        compiler.setXelatexPath("xelatex");

        bool finished = false;
        bool success = false;
        QString logOutput;

        QObject::connect(&compiler, &LatexCompiler::compilationFinished,
            [&](bool ok, const QString &, const QString &log) {
                finished = true;
                success = ok;
                logOutput = log;
            });

        compiler.compile("\\begin{tikzpicture}\n\\invalidcommand\n\\end{tikzpicture}", "", "test_error");

        QEventLoop loop;
        QTimer::singleShot(15000, &loop, &QEventLoop::quit);
        QObject::connect(&compiler, &LatexCompiler::compilationFinished, &loop, &QEventLoop::quit);
        loop.exec();

        if (!finished) { qDebug() << "FAIL: Test 5 - compilation timed out"; failed++; }
        else if (success) { qDebug() << "FAIL: Test 5 - error compilation should fail"; failed++; }
        else if (logOutput.isEmpty()) { qDebug() << "FAIL: Test 5 - log should not be empty"; failed++; }
        else qDebug() << "PASS: Test 5 - error compilation correctly failed with log";
    }

    // ──── Custom command extraction tests ────
    fprintf(stderr, "=== Starting custom command tests ===\n");
    int customTestsFailed = 0;

    // Test 6: Basic \newcommand extraction (no args)
    {
        QString code = "\\newcommand{\\mycolor}{red}\n"
                       "\\begin{tikzpicture}\n"
                       "\\draw[\\mycolor] (0,0) -- (1,1);\n"
                       "\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (cmds.isEmpty() || !cmds.contains("\\newcommand{\\mycolor}{red}"))
            { fprintf(stderr, "FAIL: Test 6 - basic newcommand not extracted\n"); customTestsFailed++; }
        else if (outCode.contains("\\newcommand"))
            { fprintf(stderr, "FAIL: Test 6 - newcommand still in output code\n"); customTestsFailed++; }
        else if (!outCode.contains("\\draw[\\mycolor]"))
            { fprintf(stderr, "FAIL: Test 6 - tikz code altered\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 6 - basic \\newcommand extraction\n");
    }

    // Test 7: \newcommand with number of args
    {
        QString code = "\\newcommand{\\myvec}[2]{\\left(#1,#2\\right)}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\newcommand{\\myvec}[2]{\\left(#1,#2\\right)}"))
            { fprintf(stderr, "FAIL: Test 7 - newcommand with args not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 7 - \\newcommand with argument count\n");
    }

    // Test 8: \newcommand* (starred)
    {
        QString code = "\\newcommand*{\\myfunc}[2]{#1+#2}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\newcommand*{\\myfunc}[2]{#1+#2}"))
            { fprintf(stderr, "FAIL: Test 8 - starred newcommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 8 - starred \\newcommand* extraction\n");
    }

    // Test 9: \renewcommand
    {
        QString code = "\\renewcommand{\\theequation}{Eq.\\arabic{equation}}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\renewcommand{\\theequation}{Eq.\\arabic{equation}}"))
            { fprintf(stderr, "FAIL: Test 9 - renewcommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 9 - \\renewcommand extraction\n");
    }

    // Test 10: \providecommand
    {
        QString code = "\\providecommand{\\mydef}{default}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\providecommand{\\mydef}{default}"))
            { fprintf(stderr, "FAIL: Test 10 - providecommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 10 - \\providecommand extraction\n");
    }

    // Test 11: \NewDocumentCommand
    {
        QString code = "\\NewDocumentCommand{\\mydraw}{ O{red} m }{\\draw[#1] #2;}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\NewDocumentCommand{\\mydraw}{ O{red} m }{\\draw[#1] #2;}"))
            { fprintf(stderr, "FAIL: Test 11 - NewDocumentCommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 11 - \\NewDocumentCommand extraction\n");
    }

    // Test 12: \NewExpandableDocumentCommand
    {
        QString code = "\\NewExpandableDocumentCommand{\\mycalc}{ m m }{#1+#2}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\NewExpandableDocumentCommand{\\mycalc}{ m m }{#1+#2}"))
            { fprintf(stderr, "FAIL: Test 12 - NewExpandableDocumentCommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 12 - \\NewExpandableDocumentCommand extraction\n");
    }

    // Test 13: \RenewDocumentCommand
    {
        QString code = "\\RenewDocumentCommand{\\mycmd}{ m }{new: #1}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\RenewDocumentCommand{\\mycmd}{ m }{new: #1}"))
            { fprintf(stderr, "FAIL: Test 13 - RenewDocumentCommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 13 - \\RenewDocumentCommand extraction\n");
    }

    // Test 14: \ProvideDocumentCommand
    {
        QString code = "\\ProvideDocumentCommand{\\mydef}{}{default}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\ProvideDocumentCommand{\\mydef}{}{default}"))
            { fprintf(stderr, "FAIL: Test 14 - ProvideDocumentCommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 14 - \\ProvideDocumentCommand extraction\n");
    }

    // Test 15: \DeclareDocumentCommand
    {
        QString code = "\\DeclareDocumentCommand{\\mycmd}{ m }{#1}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\DeclareDocumentCommand{\\mycmd}{ m }{#1}"))
            { fprintf(stderr, "FAIL: Test 15 - DeclareDocumentCommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 15 - \\DeclareDocumentCommand extraction\n");
    }

    // Test 16: Multiple custom commands
    {
        QString code = "\\newcommand{\\RED}{red}\n"
                       "\\newcommand{\\BLUE}{blue}\n"
                       "\\begin{tikzpicture}\n"
                       "\\draw[\\RED] (0,0) -- (1,1);\n"
                       "\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\RED"))
            { fprintf(stderr, "FAIL: Test 16a - first command not extracted\n"); customTestsFailed++; }
        if (!cmds.contains("\\BLUE"))
            { fprintf(stderr, "FAIL: Test 16b - second command not extracted\n"); customTestsFailed++; }
        if (cmds.count("\\newcommand") != 2)
            { fprintf(stderr, "FAIL: Test 16c - should extract 2 newcommand\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 16 - multiple custom commands\n");
    }

    // Test 17: Commands inside tikzpicture should NOT be extracted
    {
        QString code = "\\begin{tikzpicture}\n"
                       "\\newcommand{\\inside}{foo}\n"
                       "\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.isEmpty())
            { fprintf(stderr, "FAIL: Test 17 - command inside tikzpicture should not be extracted\n"); customTestsFailed++; }
        else if (!outCode.contains("\\newcommand"))
            { fprintf(stderr, "FAIL: Test 17 - outCode should still contain inner newcommand\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 17 - commands inside tikzpicture not extracted\n");
    }

    // Test 18: Commands before and after tikzpicture (only before extracted)
    {
        QString code = "\\newcommand{\\BEFORE}{1}\n"
                       "\\begin{tikzpicture}\n"
                       "\\newcommand{\\INSIDE}{2}\n"
                       "\\end{tikzpicture}\n"
                       "\\newcommand{\\AFTER}{3}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\BEFORE"))
            { fprintf(stderr, "FAIL: Test 18a - before command not extracted\n"); customTestsFailed++; }
        if (cmds.contains("\\AFTER"))
            { fprintf(stderr, "FAIL: Test 18b - after first tikzpicture command extracted incorrectly\n"); customTestsFailed++; }
        if (outCode.contains("\\BEFORE"))
            { fprintf(stderr, "FAIL: Test 18c - before command still in outCode\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 18 - only commands before first tikzpicture extracted\n");
    }

    // Test 19: Multi-line custom command definition
    {
        QString code = "\\newcommand{\\mydrawing}{%\n"
                       "  \\node[draw] at (0,0) {A};%\n"
                       "  \\node[draw] at (1,0) {B};%\n"
                       "}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\mydrawing"))
            { fprintf(stderr, "FAIL: Test 19 - multi-line command not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 19 - multi-line custom command\n");
    }

    // Test 20: Mixed old and new format
    {
        QString code = "\\newcommand{\\OLD}{old}\n"
                       "\\NewDocumentCommand{\\NEW}{}{new}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\OLD"))
            { fprintf(stderr, "FAIL: Test 20a - old format not extracted\n"); customTestsFailed++; }
        if (!cmds.contains("\\NEW"))
            { fprintf(stderr, "FAIL: Test 20b - new format not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 20 - mixed old and new format commands\n");
    }

    // Test 21: \NewCommandCopy
    {
        QString code = "\\NewCommandCopy{\\mycopy}{\\existing}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\NewCommandCopy{\\mycopy}{\\existing}"))
            { fprintf(stderr, "FAIL: Test 21 - NewCommandCopy not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 21 - \\NewCommandCopy extraction\n");
    }

    // Test 22: No custom commands (should return empty)
    {
        QString code = "\\begin{tikzpicture}\n"
                       "\\draw (0,0) -- (1,1);\n"
                       "\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.isEmpty())
            { fprintf(stderr, "FAIL: Test 22 - should return empty for no custom commands\n"); customTestsFailed++; }
        else if (outCode != code)
            { fprintf(stderr, "FAIL: Test 22 - outCode should equal input when no commands\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 22 - no custom commands returns empty\n");
    }

    // Test 23: Commands with nested braces in definition
    {
        QString code = "\\newcommand{\\nested}{{\\bfseries #1}}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\nested"))
            { fprintf(stderr, "FAIL: Test 23 - command with nested braces not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 23 - command with nested braces\n");
    }

    // Test 24: \renewcommand*
    {
        QString code = "\\renewcommand*{\\cmd}[2]{#1:#2}\n"
                       "\\begin{tikzpicture}\\end{tikzpicture}";
        QString outCode;
        QString cmds = LatexCompiler::extractCustomCommands(code, outCode);
        if (!cmds.contains("\\renewcommand*{\\cmd}[2]{#1:#2}"))
            { fprintf(stderr, "FAIL: Test 24 - starred renewcommand not extracted\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 24 - starred \\renewcommand* extraction\n");
    }

    // Test 25: wrapCode injects custom commands into preamble
    {
        LatexCompiler compiler;
        QString code = "\\begin{tikzpicture}\\draw (0,0)--(1,1);\\end{tikzpicture}";
        QString cmds = "\\newcommand{\\CMD}{test}";
        QString wrapped = compiler.wrapCode(code, "", "", "", cmds);
        // Custom commands should appear before \\begin{document}
        int docIdx = wrapped.indexOf("\\begin{document}");
        int cmdIdx = wrapped.indexOf("\\newcommand");
        if (cmdIdx < 0 || cmdIdx >= docIdx)
            { fprintf(stderr, "FAIL: Test 25 - custom commands not injected before begin{document}\n"); customTestsFailed++; }
        else fprintf(stderr, "PASS: Test 25 - wrapCode injects custom commands into preamble\n");
    }

    fprintf(stderr, "Custom command tests: %d failed\n", customTestsFailed);
    failed += customTestsFailed;

    if (failed > 0) {
        fprintf(stderr, "\n%d test(s) failed!\n", failed);
        return 1;
    }

    fprintf(stderr, "\nAll LatexCompiler tests passed!\n");
    return 0;
}
