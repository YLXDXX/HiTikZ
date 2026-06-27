#include "latex_compiler.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>

LatexCompiler::LatexCompiler(QObject *parent)
    : QObject(parent)
    , process(nullptr)
    , xelatexPath("xelatex")
    , pdfToCairoPath("pdftocairo")
{
    tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/TikzManager/";
    QDir().mkpath(tempDir);
}

LatexCompiler::~LatexCompiler()
{
    cancelCompile();
}

void LatexCompiler::setTemplateDir(const QString &dir)
{
    templateDir = dir;
}

void LatexCompiler::setXelatexPath(const QString &path)
{
    xelatexPath = path;
}

void LatexCompiler::setPdfToCairoPath(const QString &path)
{
    pdfToCairoPath = path;
}

void LatexCompiler::setTexInputs(const QString &texInputs)
{
    this->texInputs = texInputs;
}

QString LatexCompiler::xelatexCommand() const
{
    return xelatexPath;
}

QString LatexCompiler::pdfToCairoCommand() const
{
    return pdfToCairoPath;
}

QString LatexCompiler::tempDirPath() const
{
    return tempDir;
}

QString LatexCompiler::pdfPath() const
{
    return currentCompileDir.isEmpty() ? QString() : currentCompileDir + "/output.pdf";
}

QString LatexCompiler::pngPath() const
{
    return currentCompileDir.isEmpty() ? QString() : currentCompileDir + "/output.png";
}

QString LatexCompiler::svgPath() const
{
    return currentCompileDir.isEmpty() ? QString() : currentCompileDir + "/output.svg";
}

QString LatexCompiler::logPath() const
{
    return currentCompileDir.isEmpty() ? QString() : currentCompileDir + "/output.log";
}

bool LatexCompiler::checkXelatexAvailable()
{
    QProcess test;
    test.start("xelatex", QStringList() << "--version");
    test.waitForFinished(3000);
    return test.exitCode() == 0;
}

bool LatexCompiler::checkPdfToCairoAvailable()
{
    QProcess test;
    test.start("pdftocairo", QStringList() << "-v");
    test.waitForFinished(3000);
    return test.exitCode() == 0;
}

QString LatexCompiler::loadTemplate(const QString &templateId) const
{
    if (templateDir.isEmpty()) {
        return QString();
    }

    QStringList candidates = {
        templateDir + "/" + templateId + ".tex",
        templateDir + "/" + templateId,
    };

    for (const QString &path : candidates) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = QString::fromUtf8(file.readAll());
            file.close();
            return content;
        }
    }
    return QString();
}

QString LatexCompiler::wrapCode(const QString &texCode, const QString &templateId) const
{
    QString tmpl = loadTemplate(templateId);
    if (!tmpl.isEmpty() && tmpl.contains("%%% TIKZ_CODE_HERE %%%")) {
        return QString(tmpl).replace("%%% TIKZ_CODE_HERE %%%", texCode);
    }

    return QString(
        "\\documentclass[tikz, border=5pt]{standalone}\n"
        "\\usepackage{tikz}\n"
        "\\usepackage{xcolor}\n"
        "\\usetikzlibrary{calc,shapes,arrows,positioning,patterns}\n"
        "\\begin{document}\n"
        "%1\n"
        "\\end{document}\n"
    ).arg(texCode);
}

void LatexCompiler::cancelCompile()
{
    if (process && process->state() != QProcess::NotRunning) {
        process->kill();
        process->waitForFinished(3000);
    }
}

void LatexCompiler::compile(const QString &texCode, const QString &templateId, const QString &snippetId)
{
    cancelCompile();

    if (process) {
        process->deleteLater();
        process = nullptr;
    }

    currentCompileDir = tempDir + snippetId;
    QDir().mkpath(currentCompileDir);

    QString fullCode = wrapCode(texCode, templateId);

    QString texFilePath = currentCompileDir + "/output.tex";
    QFile file(texFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit compilationFinished(false, QString(), "Failed to write tex file");
        return;
    }
    file.write(fullCode.toUtf8());
    file.close();

    process = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!texInputs.isEmpty()) {
        QString currentInputs = env.value("TEXINPUTS", "");
        if (currentInputs.isEmpty()) {
            env.insert("TEXINPUTS", texInputs + ":");
        } else {
            env.insert("TEXINPUTS", texInputs + ":" + currentInputs);
        }
    }
    process->setProcessEnvironment(env);
    process->setWorkingDirectory(currentCompileDir);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, [this](int exitCode, QProcess::ExitStatus status) {
            Q_UNUSED(status);

            QString logOutput;
            QFile logFile(logPath());
            if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                logOutput = QString::fromUtf8(logFile.readAll());
                logFile.close();
            }

            if (process) {
                logOutput += process->readAllStandardOutput();
                logOutput += "\n" + process->readAllStandardError();
            }

            QString pdf = pdfPath();
            bool success = (exitCode == 0 && QFile::exists(pdf));

            emit compilationFinished(success, pdf, logOutput);
        });

    QStringList args;
    args << "-interaction=nonstopmode"
         << "-halt-on-error"
         << "-no-shell-escape"
         << "-output-directory" << currentCompileDir
         << texFilePath;

    process->start(xelatexPath, args);
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
    connect(conv, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, [this, conv, outPrefix](int exitCode, QProcess::ExitStatus) {
            QString png = outPrefix + ".png";
            bool ok = (exitCode == 0 && QFile::exists(png));
            emit conversionFinished(ok, ok ? png : QString());
            conv->deleteLater();
        });

    conv->start(pdfToCairoPath, args);
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

    QStringList args;
    args << "-svg" << pdfPath << outSvg;

    QProcess *conv = new QProcess(this);
    connect(conv, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, [this, conv, outSvg](int exitCode, QProcess::ExitStatus) {
            bool ok = (exitCode == 0 && QFile::exists(outSvg));
            emit conversionFinished(ok, ok ? outSvg : QString());
            conv->deleteLater();
        });

    conv->start(pdfToCairoPath, args);
}
