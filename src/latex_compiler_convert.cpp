#include "latex_compiler.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QEventLoop>
#include <QTimer>

bool LatexCompiler::checkXelatexAvailable() const
{
    QProcess test;
    test.setProcessEnvironment(processEnvironment());
    test.start(resolveTool(xelatexPath), QStringList() << "--version");
    if (!test.waitForStarted(3000))
        return false;
    test.waitForFinished(3000);
    return test.exitCode() == 0;
}

bool LatexCompiler::checkPdfToCairoAvailable() const
{
    QProcess test;
    test.setProcessEnvironment(processEnvironment());
    test.start(resolveTool(pdfToCairoPath), QStringList() << "-v");
    if (!test.waitForStarted(3000))
        return false;
    test.waitForFinished(3000);
    return test.exitCode() == 0;
}

bool LatexCompiler::checkInkscapeAvailable() const
{
    QProcess test;
    test.setProcessEnvironment(processEnvironment());
    test.start(resolveTool(inkscapePath), QStringList() << "--version");
    if (!test.waitForStarted(3000))
        return false;
    test.waitForFinished(3000);
    return test.exitCode() == 0;
}

void LatexCompiler::convertToPng(int dpi)
{
    convertToPng(pdfPath(), dpi);
}

void LatexCompiler::convertToPng(const QString &pdfPath, int dpi)
{
    if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
        emit conversionFinished(false, QString());
        return;
    }

    QFileInfo fi(pdfPath);
    QString outPrefix = fi.absolutePath() + "/" + fi.completeBaseName();

    QStringList args;
    args << "-png" << "-r" << QString::number(dpi) << "-singlefile" << pdfPath << outPrefix;

    QProcess *conv = new QProcess(this);
    conv->setProcessEnvironment(processEnvironment());
    connect(conv, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, [this, conv, outPrefix](int exitCode, QProcess::ExitStatus) {
            QString png = outPrefix + ".png";
            bool ok = (exitCode == 0 && QFile::exists(png));
            emit conversionFinished(ok, ok ? png : QString());
            conv->deleteLater();
        });
    connect(conv, &QProcess::errorOccurred,
        this, [this, conv](QProcess::ProcessError) {
            emit conversionFinished(false, QString());
            conv->deleteLater();
        });

    conv->start(resolveTool(pdfToCairoPath), args);
}

void LatexCompiler::convertToSvg()
{
    convertToSvg(pdfPath());
}

void LatexCompiler::convertToSvg(const QString &pdfPath)
{
    if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
        emit conversionFinished(false, QString());
        return;
    }

    QFileInfo fi(pdfPath);
    QString outSvg = fi.absolutePath() + "/" + fi.completeBaseName() + ".svg";

    QProcess *conv = new QProcess(this);
    conv->setProcessEnvironment(processEnvironment());
    connect(conv, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, [this, conv, outSvg](int exitCode, QProcess::ExitStatus) {
            bool ok = (exitCode == 0 && QFile::exists(outSvg));
            emit conversionFinished(ok, ok ? outSvg : QString());
            conv->deleteLater();
        });
    connect(conv, &QProcess::errorOccurred,
        this, [this, conv](QProcess::ProcessError) {
            emit conversionFinished(false, QString());
            conv->deleteLater();
        });

    if (svgTool_ == "inkscape") {
        QStringList args;
        args << "--export-type=svg"
             << "--pdf-poppler"
             << "--export-text-to-path"
             << "--pages=1"
             << pdfPath
             << "-o" << outSvg;
        conv->start(resolveTool(inkscapePath), args);
    } else {
        QStringList args;
        args << "-svg" << pdfPath << outSvg;
        conv->start(resolveTool(pdfToCairoPath), args);
    }
}

bool LatexCompiler::convertToSvgBlocking(const QString &pdfPath, const QString &outSvgPath)
{
    if (pdfPath.isEmpty() || !QFile::exists(pdfPath))
        return false;

    QProcess proc;
    proc.setProcessEnvironment(processEnvironment());
    if (svgTool_ == "inkscape") {
        QStringList args;
        args << "--export-type=svg"
             << "--pdf-poppler"
             << "--export-text-to-path"
             << "--pages=1"
             << pdfPath
             << "-o" << outSvgPath;
        proc.start(resolveTool(inkscapePath), args);
    } else {
        QStringList args;
        args << "-svg" << pdfPath << outSvgPath;
        proc.start(resolveTool(pdfToCairoPath), args);
    }
    proc.waitForFinished(15000);
    return (proc.exitCode() == 0 && QFile::exists(outSvgPath));
}
