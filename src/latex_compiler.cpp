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
    if (process) {
        if (process->state() != QProcess::NotRunning) {
            process->kill();
        }
        process->disconnect(this);
        process->deleteLater();
    }
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

QString LatexCompiler::wrapCode(const QString &texCode, const QString &templateId,
                                const QString &packages, const QString &tikzLibraries) const
{
    QString tmpl = loadTemplate(templateId);
    if (!tmpl.isEmpty() && tmpl.contains("%%% TIKZ_CODE_HERE %%%")) {
        tmpl = QString(tmpl).replace("%%% TIKZ_CODE_HERE %%%", texCode);
    } else {
        tmpl = QString(
            "\\documentclass[tikz, border=5pt]{standalone}\n"
            "\\usepackage{tikz}\n"
            "\\usepackage{xcolor}\n"
            "\\usepackage{ctex}\n"
            "\\usetikzlibrary{calc,shapes,arrows,positioning,patterns}\n"
            "\\begin{document}\n"
            "%1\n"
            "\\end{document}\n"
        ).arg(texCode);
    }

    QString extraPreamble;

    if (!packages.isEmpty()) {
        QStringList items;
        int bracketDepth = 0;
        int braceDepth = 0;
        int start = 0;
        for (int i = 0; i < packages.length(); ++i) {
            QChar ch = packages.at(i);
            if (ch == '{') {
                braceDepth++;
            } else if (ch == '}') {
                if (braceDepth > 0) braceDepth--;
            } else if (ch == '[' && braceDepth == 0) {
                bracketDepth++;
            } else if (ch == ']' && braceDepth == 0) {
                if (bracketDepth > 0) bracketDepth--;
            } else if (ch == ',' && bracketDepth == 0 && braceDepth == 0) {
                items.append(packages.mid(start, i - start).trimmed());
                start = i + 1;
            }
        }
        items.append(packages.mid(start).trimmed());

        for (const QString &item : items) {
            if (item.isEmpty()) continue;

            if (item.startsWith('[')) {
                int bDepth = 0;
                int cDepth = 0;
                int closeBracket = -1;
                for (int i = 0; i < item.length(); ++i) {
                    QChar ch = item.at(i);
                    if (ch == '{') cDepth++;
                    else if (ch == '}') { if (cDepth > 0) cDepth--; }
                    else if (ch == '[' && cDepth == 0) bDepth++;
                    else if (ch == ']' && cDepth == 0) {
                        bDepth--;
                        if (bDepth == 0) { closeBracket = i; break; }
                    }
                }
                if (closeBracket > 0) {
                    QString options = item.mid(1, closeBracket - 1);
                    QString pkgName = item.mid(closeBracket + 1);
                    extraPreamble += QStringLiteral("\\usepackage[%1]{%2}\n").arg(options, pkgName);
                }
            } else {
                extraPreamble += QStringLiteral("\\usepackage{%1}\n").arg(item);
            }
        }
    }

    if (!tikzLibraries.isEmpty()) {
        QStringList libs;
        QStringList parts = tikzLibraries.split(',');
        for (const QString &part : parts) {
            QString trimmed = part.trimmed();
            if (!trimmed.isEmpty())
                libs.append(trimmed);
        }
        if (!libs.isEmpty()) {
            extraPreamble += QStringLiteral("\\usetikzlibrary{%1}\n").arg(libs.join(','));
        }
    }

    if (!extraPreamble.isEmpty()) {
        int docBegin = tmpl.indexOf(QStringLiteral("\\begin{document}"));
        if (docBegin >= 0) {
            tmpl = tmpl.insert(docBegin, extraPreamble);
        }
    }

    return tmpl;
}

void LatexCompiler::cancelCompile()
{
    if (process && process->state() != QProcess::NotRunning) {
        process->kill();
    }
}

int LatexCompiler::userCodeStartLine() const
{
    return m_userCodeStartLine;
}

void LatexCompiler::compile(const QString &texCode, const QString &templateId, const QString &snippetId,
                            const QString &packages, const QString &tikzLibraries)
{
    if (!process) {
        process = new QProcess(this);
    } else {
        if (process->state() != QProcess::NotRunning)
            process->kill();
        process->disconnect();
    }

    currentCompileDir = tempDir + snippetId;
    QDir().mkpath(currentCompileDir);

    QString fullCode = wrapCode(texCode, templateId, packages, tikzLibraries);

    {
        int docBeginIdx = fullCode.indexOf(QStringLiteral("\\begin{document}"));
        if (docBeginIdx >= 0) {
            int afterBeginDoc = fullCode.indexOf('\n', docBeginIdx) + 1;
            if (afterBeginDoc > 0)
                m_userCodeStartLine = fullCode.left(afterBeginDoc).count('\n') + 1;
            else
                m_userCodeStartLine = fullCode.left(docBeginIdx).count('\n') + 1;
        } else {
            m_userCodeStartLine = 1;
        }
    }

    QString texFilePath = currentCompileDir + "/output.tex";
    QFile file(texFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit compilationFinished(false, QString(), "Failed to write tex file");
        return;
    }
    file.write(fullCode.toUtf8());
    file.close();

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
          << "-shell-escape"
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
