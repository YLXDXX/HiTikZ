#include "latex_compiler.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QEventLoop>
#include <QTimer>

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
            // \def\name<param text>{body}
            // The parameter text may contain delimiters like [ ] ( ) # and
            // digits (e.g. \def\foo[size=#1](#2,#3){...}), so skip the macro
            // name and then everything up to the body's opening '{' (TeX
            // parameter text can never contain a '{').
            if (pos < remaining.length() && remaining.at(pos) == '\\') {
                pos++; // skip backslash
                while (pos < remaining.length() && remaining.at(pos).isLetter())
                    pos++;
            }
            while (pos < remaining.length() && remaining.at(pos) != '{') {
                QChar ch = remaining.at(pos);
                if (ch == QLatin1Char('\\')
                    && pos + 1 < remaining.length()
                    && remaining.at(pos + 1).isLetter()) {
                    break;
                }
                if (ch == QLatin1Char('%'))
                    break;
                pos++;
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

            skipWs(remaining, pos);
            if (isOldCmd && pos < remaining.length() && remaining.at(pos) == '[') {
                readBalancedBrackets(remaining, pos);
                skipWs(remaining, pos);
            }
            if (isOldCmd && pos < remaining.length() && remaining.at(pos) == '[') {
                readBalancedBrackets(remaining, pos);
                skipWs(remaining, pos);
            }

            if (isDocCmd && pos < remaining.length() && remaining.at(pos) == '{') {
                readBalancedBraces(remaining, pos);
                skipWs(remaining, pos);
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
            // Skip this incomplete/unparseable definition (e.g. \newcommand{\foo}
            // without a {body}) instead of breaking the loop, so subsequent
            // custom commands are not silently discarded.
            remaining = remaining.left(cmdStart)
                        + remaining.mid(cmdStart + m.capturedLength());
        }
    }

    outCode = remaining.trimmed();

    if (commands.isEmpty()) {
        return QString();
    }

    return commands.join('\n').trimmed();
}
