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

bool LatexCompiler::checkInkscapeAvailable()
{
    QProcess test;
    test.start("inkscape", QStringList() << "--version");
    test.waitForFinished(3000);
    return test.exitCode() == 0;
}

QString LatexCompiler::loadTemplate(const QString &templateId) const
{
    if (templateDir.isEmpty() || templateId.isEmpty()) {
        return QString();
    }

    if (templateId.contains('/') || templateId.contains('\\')
        || templateId.contains("..")) {
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
                                const QString &packages, const QString &tikzLibraries,
                                const QString &customCommands) const
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
        } else {
            tmpl = extraPreamble + "\n" + tmpl;
        }
    }

    if (!customCommands.trimmed().isEmpty()) {
        int docBeginPos = tmpl.indexOf(QStringLiteral("\\begin{document}"));
        if (docBeginPos >= 0) {
            tmpl = tmpl.insert(docBeginPos, customCommands.trimmed() + "\n");
        } else {
            tmpl = customCommands.trimmed() + "\n\n" + tmpl;
        }
    }

    return tmpl;
}

QString LatexCompiler::extractCustomCommands(const QString &texCode, QString &outCode)
{
    static const QRegularExpression cmdRe(
        QStringLiteral("\\\\(?:new|renew|provide)command\\*?"
                       "|\\\\New(?:Expandable)?DocumentCommand"
                       "|\\\\RenewDocumentCommand"
                       "|\\\\ProvideDocumentCommand"
                       "|\\\\DeclareDocumentCommand"
                       "|\\\\NewCommandCopy"
                       "|\\\\pgfmathdeclarerandomlist"
                       "|\\\\definecolor"
                       "|\\\\colorlet"
                       "|\\\\tikzset"
                       "|\\\\tikzstyle"
                       "|\\\\makeatletter"
                       "|\\\\makeatother"
                       "|\\\\ctikzset"
                       "|\\\\pgfkeys"
                       "|\\\\contourlength"
                       "|\\\\usepgfplotslibrary"
                       "|\\\\pgfmathsetmacro"
                       "|\\\\pgfmathsetlength"
                       "|\\\\newlength"
                       "|\\\\newcounter"
                       "|\\\\newsavebox"
                       "|\\\\DeclareMathOperator"
                       "|\\\\DeclareRobustCommand"
                       "|\\\\pgfdeclareradialshading"
                       "|\\\\pgfdeclaredecoration"
                       "|\\\\pgfdeclareverticalshading"
                       "|\\\\tikzoption"
                       "|\\\\setlength"
                       "|\\\\sansmath"
                       "|\\\\newif"
                       "|\\\\newboolean"
                       "|\\\\setboolean"
                       "|\\\\setcounter"
                       "|\\\\PreviewEnvironment"
                       "|\\\\pgfplotsset"
                       "|\\\\def"
                       "|\\\\tikzmath"
                       "|\\\\pgfdeclarelayer"
                       "|\\\\pgfsetlayers"
                       "|\\\\tdplotsetmaincoords"
                       "|\\\\pgfmathdeclarefunction"));
    outCode = texCode;

    auto readBalancedBraces = [](const QString &s, int &pos) {
        int count = 0;
        int start = pos;
        if (pos >= s.length() || s.at(pos) != '{') return;
        pos++;
        count = 1;
        while (pos < s.length() && count > 0) {
            if (s.at(pos) == '{') count++;
            else if (s.at(pos) == '}') count--;
            pos++;
        }
    };

    auto readBalancedBrackets = [](const QString &s, int &pos) {
        int count = 0;
        if (pos >= s.length() || s.at(pos) != '[') return;
        pos++;
        count = 1;
        while (pos < s.length() && count > 0) {
            if (s.at(pos) == '[') count++;
            else if (s.at(pos) == ']') count--;
            pos++;
        }
    };

    auto skipWs = [](const QString &s, int &pos) {
        while (pos < s.length() && (s.at(pos) == ' ' || s.at(pos) == '\t' || s.at(pos) == '\n' || s.at(pos) == '\r')) {
            pos++;
        }
    };

    QStringList commands;
    QString remaining = texCode;

    while (true) {
        QRegularExpression envRe(QStringLiteral("\\\\begin\\{(?:tikzpicture|circuitikz)\\}"));
        QRegularExpressionMatch envMatch = envRe.match(remaining);
        int envStart = envMatch.hasMatch() ? envMatch.capturedStart() : -1;

        QRegularExpressionMatch m = cmdRe.match(remaining);
        if (!m.hasMatch()) break;

        int cmdStart = m.capturedStart();
        QString cmdName = m.captured();

        if (envStart >= 0 && cmdStart > envStart) break;

        int pos = cmdStart + m.capturedLength();
        skipWs(remaining, pos);

        bool isDocCmd = cmdName.endsWith("DocumentCommand") || cmdName == "\\DeclareDocumentCommand";
        bool isCopyCmd = (cmdName == "\\NewCommandCopy");
        bool isDeclRandom = (cmdName == "\\pgfmathdeclarerandomlist");
        bool isDcfColor = (cmdName == "\\definecolor");
        bool isColorlet = (cmdName == "\\colorlet");
        bool isTikzset = (cmdName == "\\tikzset");
        bool isTikzStyle = (cmdName == "\\tikzstyle");
        bool isMakeat = (cmdName == "\\makeatletter" || cmdName == "\\makeatother");
        bool isCtikzset = (cmdName == "\\ctikzset");
        bool isOldCmd = !isDocCmd && !isCopyCmd && !isDeclRandom && !isDcfColor && !isColorlet && !isTikzset && !isTikzStyle && !isMakeat && !isCtikzset;

        // Additional command flag checks
        bool isPgfkeys = (cmdName == "\\pgfkeys");
        bool isContourlength = (cmdName == "\\contourlength");
        bool isUsepgfplotslibrary = (cmdName == "\\usepgfplotslibrary");
        bool isPgfmathsetmacro = (cmdName == "\\pgfmathsetmacro");
        bool isPgfmathsetlength = (cmdName == "\\pgfmathsetlength");
        bool isNewlength = (cmdName == "\\newlength");
        bool isNewcounter = (cmdName == "\\newcounter");
        bool isNewsavebox = (cmdName == "\\newsavebox");
        bool isDeclOp = (cmdName == "\\DeclareMathOperator");
        bool isDeclRobust = (cmdName == "\\DeclareRobustCommand");
        bool isPgfradial = (cmdName == "\\pgfdeclareradialshading");
        bool isPgfdecoration = (cmdName == "\\pgfdeclaredecoration");
        bool isTikzoption = (cmdName == "\\tikzoption");
        bool isSetlength = (cmdName == "\\setlength");
        bool isSansmath = (cmdName == "\\sansmath");
        bool isNewif = (cmdName == "\\newif");
        bool isPreviewEnv = (cmdName == "\\PreviewEnvironment");
        bool isPgfplotsset = (cmdName == "\\pgfplotsset");
        bool isDef = (cmdName == "\\def");
        bool isTikzmath = (cmdName == "\\tikzmath");
        bool isPgfdeclarelayer = (cmdName == "\\pgfdeclarelayer");
        bool isPgfsetlayers = (cmdName == "\\pgfsetlayers");
        bool isNewboolean = (cmdName == "\\newboolean");
        bool isSetboolean = (cmdName == "\\setboolean");
        bool isSetcounter = (cmdName == "\\setcounter");
        bool isPgfverticalshading = (cmdName == "\\pgfdeclareverticalshading");
        bool isTdplotsetmaincoords = (cmdName == "\\tdplotsetmaincoords");
        bool isPgfmathdeclarefunction = (cmdName == "\\pgfmathdeclarefunction");

        // Re-classify: these are NOT oldCmd patterns
        bool isSpecialCmd = isPgfkeys || isContourlength || isUsepgfplotslibrary ||
                           isPgfmathsetmacro || isPgfmathsetlength ||
                           isNewlength || isNewcounter || isNewsavebox ||
                           isDeclOp || isDeclRobust ||
                           isPgfradial || isPgfdecoration || isTikzoption ||
                           isSetlength || isSansmath || isNewif ||
                           isPreviewEnv || isPgfplotsset || isDef ||
                           isTikzmath || isPgfdeclarelayer || isPgfsetlayers ||
                           isNewboolean || isSetboolean || isSetcounter ||
                           isPgfverticalshading || isTdplotsetmaincoords || isPgfmathdeclarefunction;

        if (isSpecialCmd) isOldCmd = false;

        int defStart = -1;
        int defEnd = -1;

        if (isCopyCmd) {
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isDeclRandom) {
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isDcfColor) {
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isColorlet) {
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isTikzset) {
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isCtikzset) {
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isTikzStyle) {
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            while (pos < remaining.length() && remaining.at(pos) == '+') pos++;
            while (pos < remaining.length() && remaining.at(pos) != '[') pos++;
            readBalancedBrackets(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isMakeat) {
            defEnd = pos;
            defStart = cmdStart;
        } else if (isPgfkeys || isContourlength || isUsepgfplotslibrary || isPgfplotsset) {
            // Single brace arg: \pgfkeys{...}, \contourlength{N}, \usepgfplotslibrary{...}, \pgfplotsset{...}
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isPgfmathsetmacro || isPgfmathsetlength) {
            // Two brace args: \pgfmathsetmacro{\name}{expr}
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isNewlength || isNewcounter || isNewsavebox) {
            // One brace arg: \newlength{\name}, \newcounter{name}, \newsavebox{\name}
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isTikzoption) {
            // Two brace args: \tikzoption{name}{code}
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isPgfradial) {
            // Optional bracket + 2 brace args: \pgfdeclareradialshading[opts]{name}{spec}{spec}
            if (pos < remaining.length() && remaining.at(pos) == '[') {
                readBalancedBrackets(remaining, pos);
                skipWs(remaining, pos);
            }
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            // May have 3rd brace
            if (pos < remaining.length() && remaining.at(pos) == '{') {
                readBalancedBraces(remaining, pos);
            }
            defEnd = pos;
            defStart = cmdStart;
        } else if (isPgfdecoration) {
            // Complex: \pgfdeclaredecoration{name}{initial}{states...}
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isSetlength) {
            // Special: \setlength\macro{value}
            if (pos < remaining.length() && remaining.at(pos) == '\\') {
                while (pos < remaining.length() && remaining.at(pos) != '[' && remaining.at(pos) != '{' && remaining.at(pos) != ' ' && remaining.at(pos) != '\t') {
                    pos++;
                }
                skipWs(remaining, pos);
            }
            if (pos < remaining.length() && remaining.at(pos) == '{') {
                readBalancedBraces(remaining, pos);
            }
            defEnd = pos;
            defStart = cmdStart;
        } else if (isSansmath) {
            // No args: \sansmath
            defEnd = pos;
            defStart = cmdStart;
        } else if (isNewif) {
            // Special: \newif\ifname
            if (pos < remaining.length() && remaining.at(pos) == '\\') {
                while (pos < remaining.length() && remaining.at(pos).isLetter()) {
                    pos++;
                }
            }
            defEnd = pos;
            defStart = cmdStart;
        } else if (isPreviewEnv) {
            // One brace arg: \PreviewEnvironment{name}
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isDef) {
            // \def\name args{body}
            if (pos < remaining.length() && remaining.at(pos) == '\\') {
                while (pos < remaining.length() && remaining.at(pos) != '[' && remaining.at(pos) != '{') {
                    pos++;
                }
            }
            if (pos < remaining.length() && remaining.at(pos) == '{') {
                defStart = pos;
                readBalancedBraces(remaining, pos);
                defEnd = pos;
            }
        } else if (isTikzmath || isPgfdeclarelayer || isPgfsetlayers) {
            // Single brace arg: \tikzmath{...}, \pgfdeclarelayer{...}, \pgfsetlayers{...}
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isDeclOp) {
            // Two brace args: \DeclareMathOperator{\name}{display}
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isDeclRobust) {
            // Like \newcommand: \DeclareRobustCommand{\name}[N]{def}
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            if (pos < remaining.length() && remaining.at(pos) == '[') {
                readBalancedBrackets(remaining, pos);
                skipWs(remaining, pos);
            }
            if (pos < remaining.length() && remaining.at(pos) == '{') {
                defStart = pos;
                readBalancedBraces(remaining, pos);
                defEnd = pos;
            }
        } else if (isNewboolean) {
            // One brace arg: \newboolean{name}
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isSetboolean || isSetcounter) {
            // Two brace args: \setboolean{name}{value}, \setcounter{name}{value}
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isPgfverticalshading) {
            // Complex: \pgfdeclareverticalshading[opts]{name}{point1}{point2}{...}
            if (pos < remaining.length() && remaining.at(pos) == '[') {
                readBalancedBrackets(remaining, pos);
                skipWs(remaining, pos);
            }
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            // May have more braces
            while (pos < remaining.length() && remaining.at(pos) == '{') {
                readBalancedBraces(remaining, pos);
                if (pos < remaining.length()) skipWs(remaining, pos);
            }
            defEnd = pos;
            defStart = cmdStart;
        } else if (isTdplotsetmaincoords) {
            // Three brace args: \tdplotsetmaincoords{theta}{phi}{psi}
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else if (isPgfmathdeclarefunction) {
            // Complex: \pgfmathdeclarefunction{name}{N}{definition}
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            skipWs(remaining, pos);
            readBalancedBraces(remaining, pos);
            defEnd = pos;
            defStart = cmdStart;
        } else {
            if (pos < remaining.length() && remaining.at(pos) == '{') {
                readBalancedBraces(remaining, pos);
            } else if (pos < remaining.length() && remaining.at(pos) == '\\') {
                while (pos < remaining.length() && remaining.at(pos) != '[' && remaining.at(pos) != '{') {
                    pos++;
                }
            }

            if (isOldCmd && pos < remaining.length() && remaining.at(pos) == '[') {
                readBalancedBrackets(remaining, pos);
            }
            if (isOldCmd && pos < remaining.length() && remaining.at(pos) == '[') {
                readBalancedBrackets(remaining, pos);
            }

            if (isDocCmd && pos < remaining.length() && remaining.at(pos) == '{') {
                readBalancedBraces(remaining, pos);
            }

            if (pos < remaining.length() && remaining.at(pos) == '{') {
                defStart = pos;
                readBalancedBraces(remaining, pos);
                defEnd = pos;
            }
        }

        if (defEnd > defStart) {
            QString fullCmd = remaining.mid(cmdStart, defEnd - cmdStart);
            commands.append(fullCmd);
            int cleanStart = defEnd;
            bool trimmedNewlineAfter = false;
            while (cleanStart < remaining.length() && (remaining.at(cleanStart) == ' ' || remaining.at(cleanStart) == '\t' || remaining.at(cleanStart) == '\n' || remaining.at(cleanStart) == '\r')) {
                if (remaining.at(cleanStart) == '\n' || remaining.at(cleanStart) == '\r')
                    trimmedNewlineAfter = true;
                cleanStart++;
            }
            int cleanEnd = cmdStart;
            bool trimmedNewlineBefore = false;
            while (cleanEnd > 0 && (remaining.at(cleanEnd - 1) == ' ' || remaining.at(cleanEnd - 1) == '\t' || remaining.at(cleanEnd - 1) == '\n' || remaining.at(cleanEnd - 1) == '\r')) {
                if (remaining.at(cleanEnd - 1) == '\n' || remaining.at(cleanEnd - 1) == '\r')
                    trimmedNewlineBefore = true;
                cleanEnd--;
            }
            // Preserve a newline separator when we removed one from either side,
            // to prevent LaTeX comment lines from merging with the next command
            QString separator;
            if (trimmedNewlineBefore || trimmedNewlineAfter)
                separator = QStringLiteral("\n");
            remaining = remaining.left(cleanEnd) + separator + remaining.mid(cleanStart);
        } else {
            break;
        }
    }

    outCode = remaining.trimmed();

    if (commands.isEmpty()) {
        return QString();
    }

    return commands.join('\n').trimmed();
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

    process->start(program, args);
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
    connect(conv, &QProcess::errorOccurred,
        this, [this, conv](QProcess::ProcessError) {
            emit conversionFinished(false, QString());
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

    QProcess *conv = new QProcess(this);
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
        conv->start(inkscapePath, args);
    } else {
        QStringList args;
        args << "-svg" << pdfPath << outSvg;
        conv->start(pdfToCairoPath, args);
    }
}

bool LatexCompiler::convertToSvgBlocking(const QString &pdfPath, const QString &outSvgPath)
{
    if (pdfPath.isEmpty() || !QFile::exists(pdfPath))
        return false;

    QProcess proc;
    if (svgTool_ == "inkscape") {
        QStringList args;
        args << "--export-type=svg"
             << "--pdf-poppler"
             << "--export-text-to-path"
             << "--pages=1"
             << pdfPath
             << "-o" << outSvgPath;
        proc.start(inkscapePath, args);
    } else {
        QStringList args;
        args << "-svg" << pdfPath << outSvgPath;
        proc.start(pdfToCairoPath, args);
    }
    proc.waitForFinished(15000);
    return (proc.exitCode() == 0 && QFile::exists(outSvgPath));
}
