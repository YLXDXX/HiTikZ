#include "latex_compiler.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QEventLoop>
#include <QTimer>

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
                    QString pkgName = item.mid(closeBracket + 1).trimmed();
                    if (pkgName.isEmpty()) {
                        // "[opts]" with no package name — nothing usable to emit.
                        qWarning() << "Malformed package (options without a package "
                                      "name), skipping:" << item;
                    } else {
                        extraPreamble += QStringLiteral("\\usepackage[%1]{%2}\n")
                                             .arg(options, pkgName);
                    }
                } else {
                    // Unbalanced '[' (missing ']'): don't silently drop the
                    // package — treat the remainder after '[' as a plain package
                    // name so the failure is at least visible/compilable.
                    QString pkgName = item.mid(1).trimmed();
                    qWarning() << "Malformed package syntax (missing ']'), treating "
                                  "as plain package:" << item;
                    if (!pkgName.isEmpty())
                        extraPreamble += QStringLiteral("\\usepackage{%1}\n").arg(pkgName);
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
