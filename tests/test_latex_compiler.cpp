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

    if (failed > 0) {
        qDebug() << "\n" << failed << "test(s) failed!";
        return 1;
    }

    qDebug() << "\nAll LatexCompiler tests passed!";
    return 0;
}
