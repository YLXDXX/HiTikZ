#include "latex_compiler.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QEventLoop>
#include <QTimer>

LatexCompiler::LatexCompiler(QObject *parent)
    : QObject(parent)
    , process(nullptr)
    , xelatexPath("xelatex")
    , pdfToCairoPath("pdftocairo")
    , inkscapePath("inkscape")
    , svgTool_("pdftocairo")
{
    tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/TikzManager/";
    QDir().mkpath(tempDir);
}

LatexCompiler::~LatexCompiler()
{
    if (process) {
        if (process->state() != QProcess::NotRunning) {
            process->kill();
            process->waitForFinished(1000);
        }
        process->disconnect(this);
        process->deleteLater();
    }

    // convertToPng()/convertToSvg() spawn async QProcess children. If the
    // compiler is destroyed while one is still running, Qt warns ("QProcess:
    // Destroyed while process still running") and the child may be orphaned.
    // Kill and reap any that remain.
    const QList<QProcess*> convProcs = findChildren<QProcess*>();
    for (QProcess *p : convProcs) {
        if (p == process) continue;
        p->disconnect(this);
        if (p->state() != QProcess::NotRunning) {
            p->kill();
            p->waitForFinished(1000);
        }
        p->deleteLater();
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

void LatexCompiler::setInkscapePath(const QString &path)
{
    inkscapePath = path;
}

void LatexCompiler::setSvgTool(const QString &tool)
{
    svgTool_ = tool;
}

void LatexCompiler::setTexInputs(const QString &texInputs)
{
    this->texInputs = texInputs;
}

QProcessEnvironment LatexCompiler::processEnvironment() const
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!texInputs.isEmpty()) {
        const QString sep = QStringLiteral(":");

        // Prepend the configured directories to PATH so bundled tools
        // (xelatex, pdftocairo, inkscape) resolve even when launched from a
        // desktop environment that provides only a minimal PATH.
        const QString path = env.value(QStringLiteral("PATH"));
        env.insert(QStringLiteral("PATH"),
                   path.isEmpty() ? texInputs : texInputs + sep + path);

        // Also expose them to TEXINPUTS so local input files are found.
        const QString currentInputs = env.value(QStringLiteral("TEXINPUTS"));
        env.insert(QStringLiteral("TEXINPUTS"),
                   currentInputs.isEmpty() ? texInputs + sep
                                           : texInputs + sep + currentInputs);
    }
    return env;
}

QString LatexCompiler::resolveTool(const QString &name) const
{
    if (name.isEmpty())
        return name;

    // Explicit path given — use as-is.
    if (name.contains(QLatin1Char('/')))
        return name;

    // Search the user-configured directories first, then the system PATH.
    // QProcess's own PATH lookup on Unix does not reliably honour the PATH set
    // via setProcessEnvironment(), so resolve to an absolute path here.
    if (!texInputs.isEmpty()) {
        const QStringList extra = texInputs.split(QLatin1Char(':'), Qt::SkipEmptyParts);
        const QString found = QStandardPaths::findExecutable(name, extra);
        if (!found.isEmpty())
            return found;
    }
    const QString sys = QStandardPaths::findExecutable(name);
    return sys.isEmpty() ? name : sys;
}

QString LatexCompiler::xelatexCommand() const
{
    return xelatexPath;
}

QString LatexCompiler::pdfToCairoCommand() const
{
    return pdfToCairoPath;
}

QString LatexCompiler::inkscapeCommand() const
{
    return inkscapePath;
}

QString LatexCompiler::svgTool() const
{
    return svgTool_;
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

void LatexCompiler::cancelCompile()
{
    if (process && process->state() != QProcess::NotRunning) {
        process->kill();
    }
}

bool LatexCompiler::compileBlocking(const QString &texCode, const QString &templateId, const QString &snippetId,
                                     const QString &packages, const QString &tikzLibraries,
                                     int timeoutMs, QString &outPdfPath, QString &outLog,
                                     const QString &compileCommand)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    bool success = false;
    QString pdfPath, log;

    QMetaObject::Connection conn = connect(this, &LatexCompiler::compilationFinished,
        [&](bool s, const QString &pdf, const QString &l) {
            success = s;
            pdfPath = pdf;
            log = l;
            loop.quit();
        });
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    compile(texCode, templateId, snippetId, packages, tikzLibraries, compileCommand);
    timer.start(timeoutMs);
    loop.exec();

    disconnect(conn);

    if (!timer.isActive()) {
        cancelCompile();
        outLog = QStringLiteral("Compilation timed out after %1 ms").arg(timeoutMs);
        outPdfPath.clear();
        return false;
    }
    timer.stop();

    outPdfPath = pdfPath;
    outLog = log;
    return success;
}

int LatexCompiler::userCodeStartLine() const
{
    return m_userCodeStartLine;
}

void LatexCompiler::compile(const QString &texCode, const QString &templateId, const QString &snippetId,
                            const QString &packages, const QString &tikzLibraries,
                            const QString &compileCommand)
{
    if (!process) {
        process = new QProcess(this);
    } else {
        if (process->state() != QProcess::NotRunning) {
            process->kill();
            process->waitForFinished(3000);
        }
        process->disconnect();
    }

    currentCompileDir = tempDir + snippetId;
    QDir().mkpath(currentCompileDir);

    QString cleanedCode;
    QString customCmds = extractCustomCommands(texCode, cleanedCode);

    QString fullCode = wrapCode(cleanedCode, templateId, packages, tikzLibraries, customCmds);

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

    process->setProcessEnvironment(processEnvironment());
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

    connect(process, &QProcess::errorOccurred,
        this, [this](QProcess::ProcessError) {
            QString logOutput;
            if (process) {
                logOutput = process->readAllStandardOutput();
                logOutput += "\n" + process->readAllStandardError();
            }
            emit compilationFinished(false, QString(), logOutput.isEmpty()
                ? QStringLiteral("Process failed to start") : logOutput);
        });

    QString program;
    QStringList userArgs;

    QString trimmedCmd = compileCommand.trimmed();
    if (!trimmedCmd.isEmpty()) {
        QStringList parts = QProcess::splitCommand(trimmedCmd);
        if (!parts.isEmpty()) {
            program = parts.takeFirst();
            userArgs = parts;
        }
    }

    if (program.isEmpty()) {
        program = xelatexPath;
        userArgs = QStringList() << "-interaction=nonstopmode"
                                 << "-halt-on-error"
                                 << "-shell-escape";
    }

    QStringList args = userArgs;
    args << "-output-directory" << currentCompileDir
         << texFilePath;

    m_lastFullCommand = program;
    for (const QString &a : args) {
        m_lastFullCommand += " " + a;
    }

    process->start(resolveTool(program), args);
}
