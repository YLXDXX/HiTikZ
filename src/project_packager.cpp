#include "project_packager.h"
#include "latex_compiler.h"
#include "link_manager.h"
#include "snippet_manager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <algorithm>

namespace {

// A destination-directory entry occupies a number group when it equals the
// zero-padded stem or starts with "stem." / "stem_" — i.e. the PDF itself
// (014.pdf), the source document (014.tex) and the related pictures
// (014_a.png) of an earlier packaging run share one number.
bool entryBelongsToStem(const QString &entry, const QString &stem)
{
    if (!entry.startsWith(stem))
        return false;
    if (entry.size() == stem.size())
        return true;
    const QChar next = entry.at(stem.size());
    return next == QLatin1Char('.') || next == QLatin1Char('_');
}

QStringList entriesBelongingToStem(const QString &destAbs, const QString &stem)
{
    QStringList out;
    const QDir dir(destAbs);
    if (!dir.exists())
        return out;
    // QDir::Files alone may skip dangling symlinks; QDir::System keeps them.
    const QStringList entries = dir.entryList(QDir::Files | QDir::System);
    for (const QString &entry : entries) {
        if (entryBelongsToStem(entry, stem))
            out.append(entry);
    }
    return out;
}

// True when the line prefix contains an unescaped '%' (LaTeX comment): the
// command on this line is commented out and must not be touched. An escaped
// "\%" does not start a comment.
bool isCommentedOut(const QString &linePrefix)
{
    int backslashes = 0;
    for (const QChar &ch : linePrefix) {
        if (ch == QLatin1Char('%') && backslashes % 2 == 0)
            return true;
        if (ch == QLatin1Char('\\'))
            ++backslashes;
        else
            backslashes = 0;
    }
    return false;
}

QString expandHome(const QString &path)
{
    if (path == QLatin1Char('~') || path.startsWith(QStringLiteral("~/")))
        return QDir::homePath() + path.mid(1);
    return path;
}

QString stripTrailingSlashes(QString path)
{
    while (path.size() > 1 && path.endsWith(QLatin1Char('/')))
        path.chop(1);
    return path;
}

// Replace @@var@@ placeholders with the defaults declared by
// "% @param: var=default" comment lines (same logic as the GUI's
// resolveParamsFromCode — the CLI has no live parameter widgets).
QString applyParamDefaults(const QString &code)
{
    QString result = code;
    static const QRegularExpression re(QStringLiteral("%\\s*@param:\\s*(\\w+)=(\\S+)"));
    QRegularExpressionMatchIterator it = re.globalMatch(code);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        result.replace(QStringLiteral("@@%1@@").arg(m.captured(1)), m.captured(2));
    }
    return result;
}

// Locate the snippet whose 「复制链接」 produced the given link file name.
// Primary route: follow the symlink to <snippetDir>/preview.pdf and read the
// directory's meta.json. Fallback: scan snippet / preset meta.json files for
// a matching linkedPdf field (covers filesystems where the link ended up as
// a plain copy).
Snippet findOwningSnippet(const LinkManager &links, SnippetManager &mgr,
                          const QString &linkName)
{
    const QFileInfo linkInfo(links.linkFilePath(linkName));
    if (linkInfo.isSymLink()) {
        QString target = linkInfo.symLinkTarget();
        if (!QFileInfo(target).isAbsolute())
            target = linkInfo.absolutePath() + QLatin1Char('/') + target;
        const QDir targetDir = QFileInfo(target).absoluteDir();
        const QString metaPath = targetDir.filePath(QStringLiteral("meta.json"));
        if (QFile::exists(metaPath)) {
            QFile metaFile(metaPath);
            if (metaFile.open(QIODevice::ReadOnly)) {
                const QJsonDocument doc = QJsonDocument::fromJson(metaFile.readAll());
                metaFile.close();
                if (doc.isObject()) {
                    const QString id = doc.object().value(QStringLiteral("id")).toString();
                    const Snippet s = mgr.loadSnippet(id);
                    if (!s.id.isEmpty() && (s.linkedPdf.isEmpty() || s.linkedPdf == linkName))
                        return s;
                }
            }
        }
    }

    const QList<Snippet> snippets = mgr.getAllSnippets(false);
    for (const Snippet &s : snippets) {
        if (s.linkedPdf == linkName)
            return mgr.loadSnippet(s.id);
    }
    const QList<Snippet> presets = mgr.getAllPresets(false);
    for (const Snippet &s : presets) {
        if (s.linkedPdf == linkName)
            return mgr.loadPreset(s.id);
    }
    return Snippet();
}

#ifdef RESOURCE_DIR
#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define STRINGIFY_IMPL(x) #x
// The GUI copies the bundled templates to the user data directory at
// startup; the CLI must do the same so snippets selecting a template get
// wrapped with it (mirrors SettingsDialog::ensureTemplatesCopied).
void ensureTemplatesCopied(const QString &templateDir)
{
    QDir().mkpath(templateDir);
    QString resourceDir = QStringLiteral(STRINGIFY(RESOURCE_DIR));
    if (!QDir(resourceDir).exists())
        resourceDir = QStringLiteral(STRINGIFY(INSTALL_RESOURCE_DIR));
    const QDir resDir(resourceDir + QStringLiteral("/templates"));
    if (!resDir.exists())
        return;
    const QStringList templates = resDir.entryList(QStringList() << QStringLiteral("*.tex"),
                                                   QDir::Files);
    for (const QString &tpl : templates) {
        const QString destPath = templateDir + tpl;
        if (!QFile::exists(destPath))
            QFile::copy(resDir.filePath(tpl), destPath);
    }
}
#endif

} // namespace

QString ProjectPackager::formatNumber(int number, int width)
{
    if (number < 1)
        number = 1;
    if (width < 2)
        width = 2;
    return QStringLiteral("%1").arg(number, width, 10, QLatin1Char('0'));
}

QStringList ProjectPackager::collectLinkFiles(const QString &linkDirAbs)
{
    QStringList names;
    const QDir dir(linkDirAbs);
    if (!dir.exists())
        return names;
    static const QRegularExpression re(QStringLiteral("^(\\d+)\\.pdf$"));
    const QStringList entries = dir.entryList(QDir::Files | QDir::System);
    QList<QPair<int, QString>> found;
    for (const QString &entry : entries) {
        const QRegularExpressionMatch m = re.match(entry);
        if (m.hasMatch())
            found.append(qMakePair(m.captured(1).toInt(), entry));
    }
    std::sort(found.begin(), found.end(),
              [](const QPair<int, QString> &a, const QPair<int, QString> &b) {
                  return a.first < b.first;
              });
    for (const auto &p : found)
        names.append(p.second);
    return names;
}

QStringList ProjectPackager::expandTexArgument(const QString &texArg)
{
    const QString arg = expandHome(texArg);
    const QFileInfo info(arg);
    if (info.isDir()) {
        const QDir dir(arg);
        QStringList out;
        const QStringList names = dir.entryList(QStringList() << QStringLiteral("*.tex"),
                                                QDir::Files, QDir::Name);
        for (const QString &name : names)
            out.append(dir.absoluteFilePath(name));
        return out;
    }
    if (arg.contains(QLatin1Char('*')) || arg.contains(QLatin1Char('?'))) {
        const QDir dir(info.absolutePath(), info.fileName(), QDir::Name, QDir::Files);
        QStringList out;
        const QStringList names = dir.entryList();
        for (const QString &name : names)
            out.append(dir.absoluteFilePath(name));
        return out;
    }
    return QStringList() << info.absoluteFilePath();
}

QString ProjectPackager::rewriteIncludes(const QString &content,
                                         const std::function<QString(const QString &)> &matcher,
                                         int *count)
{
    if (count)
        *count = 0;

    // Group 1: "\includegraphics[*][options]{" — group 2: the path —
    // group 3: "}". Only group 2 is ever replaced. A space may precede the
    // optional-argument bracket or the opening brace.
    static const QRegularExpression re(
        QStringLiteral("(\\\\includegraphics(?:\\*)?[ \\t]*(?:\\[[^\\]\\r\\n]*\\])?[ \\t]*\\{)"
                       "([^{}\\r\\n]+)(\\})"));

    QString result = content;

    // Collect all matches first, then replace back-to-front so earlier
    // offsets stay valid.
    QList<QRegularExpressionMatch> matches;
    QRegularExpressionMatchIterator it = re.globalMatch(result);
    while (it.hasNext())
        matches.prepend(it.next());

    for (const QRegularExpressionMatch &m : matches) {
        const QString rawPath = m.captured(2).trimmed();
        if (rawPath.isEmpty())
            continue;

        const int lineStart = result.lastIndexOf(QLatin1Char('\n'), m.capturedStart(1) - 1) + 1;
        const QString linePrefix = result.mid(lineStart, m.capturedStart(1) - lineStart);
        if (isCommentedOut(linePrefix))
            continue;

        const QString newPath = matcher(rawPath);
        if (newPath.isEmpty())
            continue;

        result.replace(m.capturedStart(2), m.capturedLength(2), newPath);
        if (count)
            (*count)++;
    }
    return result;
}

bool ProjectPackager::parsePackArgs(const QStringList &args, PackOptions &opts,
                                    QString &error, QString &usage)
{
    usage = QStringLiteral(
        "用法: hitikz pack [选项] <TeX文件或目录> <目标目录>\n"
        "\n"
        "将「复制链接」生成的图片从链接图片目录复制到 LaTeX 文档项目，\n"
        "并把相关 .tex 文件中的 \\includegraphics 引用改为新路径。\n"
        "\n"
        "必选参数:\n"
        "  <TeX文件或目录>  需要处理的 TeX 文件名（支持 * 通配符）或目录名\n"
        "                   （目录名时处理该目录下的所有 .tex 文件）\n"
        "  <目标目录>       保存 PDF 图片（及可选源码）的目录\n"
        "\n"
        "可选参数:\n"
        "  --link-dir <目录>     链接图片所在目录（默认读取程序设置中的链接图片目录）\n"
        "  --name-format <格式>  新文件名格式: 01 / 001 / 0001（默认 001）\n"
        "  --overwrite           覆盖目标目录中的同名文件（默认跳过并顺延编号）\n"
        "  --copy-sources        同时复制完整 TeX 源码及关联图片（默认只复制 PDF）\n"
        "  --help                显示本帮助\n");

    opts = PackOptions();
    QStringList positionals;
    bool help = false;

    for (int i = 0; i < args.size(); ++i) {
        const QString arg = args.at(i);
        if (arg == QStringLiteral("--help") || arg == QStringLiteral("-h")) {
            help = true;
        } else if (arg == QStringLiteral("--link-dir")) {
            if (i + 1 >= args.size()) {
                error = QStringLiteral("--link-dir 需要一个目录参数");
                return false;
            }
            opts.linkDir = args.at(++i);
        } else if (arg.startsWith(QStringLiteral("--link-dir="))) {
            opts.linkDir = arg.mid(QStringLiteral("--link-dir=").size());
        } else if (arg == QStringLiteral("--name-format")
                   || arg.startsWith(QStringLiteral("--name-format="))) {
            QString fmt;
            if (arg == QStringLiteral("--name-format")) {
                if (i + 1 >= args.size()) {
                    error = QStringLiteral("--name-format 需要一个参数（01 / 001 / 0001）");
                    return false;
                }
                fmt = args.at(++i);
            } else {
                fmt = arg.mid(QStringLiteral("--name-format=").size());
            }
            if (fmt == QStringLiteral("01")) {
                opts.nameWidth = 2;
            } else if (fmt == QStringLiteral("001")) {
                opts.nameWidth = 3;
            } else if (fmt == QStringLiteral("0001")) {
                opts.nameWidth = 4;
            } else {
                error = QStringLiteral("无效的命名格式: %1（可选: 01 / 001 / 0001）").arg(fmt);
                return false;
            }
        } else if (arg == QStringLiteral("--overwrite")) {
            opts.overwrite = true;
        } else if (arg == QStringLiteral("--copy-sources")) {
            opts.copySources = true;
        } else if (arg.startsWith(QLatin1Char('-'))) {
            error = QStringLiteral("未知选项: %1").arg(arg);
            return false;
        } else {
            positionals.append(arg);
        }
    }

    if (help) {
        opts.showHelp = true;
        return true;
    }

    if (positionals.size() != 2) {
        error = QStringLiteral("需要且仅需要两个位置参数: <TeX文件或目录> <目标目录>");
        return false;
    }
    opts.texArg = positionals.at(0);
    opts.destDir = stripTrailingSlashes(positionals.at(1));
    if (opts.texArg.trimmed().isEmpty()) {
        error = QStringLiteral("TeX 文件或目录参数不能为空");
        return false;
    }
    if (opts.destDir.isEmpty()) {
        error = QStringLiteral("目标目录不能为空");
        return false;
    }
    return true;
}

ProjectPackager::PackResult ProjectPackager::pack(const PackOptions &opts)
{
    PackResult res;

    // ── Link directory ─────────────────────────────────────────────────
    QString configuredLinkDir = opts.linkDir.trimmed();
    if (configuredLinkDir.isEmpty())
        configuredLinkDir = LinkManager::settingDir();
    const LinkManager links(configuredLinkDir);
    const QString linkDirAbs = links.dirPath();

    if (!QDir(linkDirAbs).exists()) {
        res.errorMessage = QStringLiteral("链接图片目录不存在: %1").arg(linkDirAbs);
        return res;
    }

    // ── Destination directory ──────────────────────────────────────────
    const QString destConfigured = stripTrailingSlashes(opts.destDir);
    const QString destAbs = stripTrailingSlashes(expandHome(destConfigured));
    if (destConfigured.isEmpty()) {
        res.errorMessage = QStringLiteral("目标目录不能为空");
        return res;
    }
    if (!QDir().mkpath(destAbs)) {
        res.errorMessage = QStringLiteral("无法创建目标目录: %1").arg(destAbs);
        return res;
    }

    // ── TeX files ──────────────────────────────────────────────────────
    res.texFiles = expandTexArgument(opts.texArg);
    if (res.texFiles.isEmpty()) {
        res.errorMessage = QStringLiteral("没有找到 TeX 文件: %1").arg(opts.texArg);
        return res;
    }
    for (const QString &texFile : res.texFiles) {
        if (!QFileInfo(texFile).isFile()) {
            res.errorMessage = QStringLiteral("TeX 文件不存在: %1").arg(texFile);
            return res;
        }
    }

    // ── Link files to copy ─────────────────────────────────────────────
    res.linkFiles = collectLinkFiles(linkDirAbs);
    if (res.linkFiles.isEmpty()) {
        res.messages.append(QStringLiteral("链接目录中没有找到可复制的图片: %1").arg(linkDirAbs));
        res.ok = true;
        return res;
    }

    res.messages.append(QStringLiteral("链接图片目录: %1").arg(configuredLinkDir));
    res.messages.append(QStringLiteral("目标目录: %1").arg(destConfigured));

    bool anyError = false;
    QMap<QString, QString> newNameOf; // link name -> new name (successful copies only)
    int number = 1;

    for (const QString &linkName : res.linkFiles) {
        QString stem;
        if (opts.overwrite) {
            // Overwrite mode: number sequentially from 01 regardless of the
            // files already present (they get replaced, as documented).
            stem = formatNumber(number, opts.nameWidth);
        } else {
            while (true) {
                const QString candidate = formatNumber(number, opts.nameWidth);
                const QStringList colliding = entriesBelongingToStem(destAbs, candidate);
                if (colliding.isEmpty())
                    break;
                for (const QString &entry : colliding)
                    res.messages.append(QStringLiteral("跳过已存在的文件: %1").arg(entry));
                ++number;
            }
            stem = formatNumber(number, opts.nameWidth);
        }
        ++number;

        const QString newName = stem + QStringLiteral(".pdf");
        res.messages.append(QStringLiteral("复制: %1 -> %2").arg(linkName, newName));

        const QString srcPath = links.linkFilePath(linkName);
        const QString dstPath = destAbs + QLatin1Char('/') + newName;

        const QFileInfo dstInfo(dstPath);
        if (opts.overwrite && (dstInfo.exists() || dstInfo.isSymLink())) {
            res.messages.append(QStringLiteral("覆盖已存在的文件: %1").arg(newName));
            QFile::remove(dstPath);
        }

        if (!QFile::copy(srcPath, dstPath)) {
            res.messages.append(QStringLiteral("复制失败: %1（源文件不可读或已失效）").arg(linkName));
            anyError = true;
            continue;
        }
        newNameOf.insert(linkName, newName);
    }

    // ── Optional: full source documents + related pictures ─────────────
    if (opts.copySources) {
        SnippetManager mgr;
        LatexCompiler compiler;
        const QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        compiler.setTemplateDir(dataLocation + QStringLiteral("/templates/"));

        for (const QString &linkName : res.linkFiles) {
            if (!newNameOf.contains(linkName))
                continue;

            const QString newName = newNameOf.value(linkName);
            const QString stem = newName.left(newName.length() - QStringLiteral(".pdf").length());

            const Snippet s = findOwningSnippet(links, mgr, linkName);
            if (s.id.isEmpty()) {
                res.messages.append(QStringLiteral("警告: 未找到 %1 对应的源码片段，跳过源码复制").arg(linkName));
                anyError = true;
                continue;
            }

            // Build the full compilable document exactly like the toolbar
            // 「复制文件」 action.
            QString code = applyParamDefaults(s.code);
            static const QRegularExpression paramLine(
                QStringLiteral("^%\\s*@param:.*(\n|\r\n?)?"),
                QRegularExpression::MultilineOption);
            code.remove(paramLine);
            QString cleanedCode;
            const QString customCmds = LatexCompiler::extractCustomCommands(code, cleanedCode);
            QString fullDoc = compiler.wrapCode(cleanedCode, s.templateId, s.packages,
                                                s.tikzLibraries, customCmds);
            fullDoc = LatexCompiler::metadataHeader(s.name, s.description,
                                                    s.tags.join(QStringLiteral(", ")))
                      + fullDoc;

            // The snippet's pictures move next to the PDF under stem-prefixed
            // names (014_a.png ...); adjust the document's own references.
            const QStringList images = mgr.getSnippetImagePaths(s.id);
            QMap<QString, QString> imageNameOf;
            for (const QString &img : images) {
                const QString orig = QFileInfo(img).fileName();
                imageNameOf.insert(orig, stem + QLatin1Char('_') + orig);
            }
            if (!imageNameOf.isEmpty()) {
                fullDoc = rewriteIncludes(fullDoc, [&imageNameOf](const QString &rawPath) -> QString {
                    return imageNameOf.value(QFileInfo(rawPath).fileName());
                });
            }

            // Write the source document.
            const QString texName = stem + QStringLiteral(".tex");
            const QString texPath = destAbs + QLatin1Char('/') + texName;
            const QFileInfo texInfo(texPath);
            if (opts.overwrite && (texInfo.exists() || texInfo.isSymLink())) {
                res.messages.append(QStringLiteral("覆盖已存在的文件: %1").arg(texName));
                QFile::remove(texPath);
            }
            QFile texFile(texPath);
            if (!texFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                res.messages.append(QStringLiteral("写入失败: %1").arg(texName));
                anyError = true;
            } else {
                texFile.write(fullDoc.toUtf8());
                texFile.close();
                res.messages.append(QStringLiteral("复制源码: %1").arg(texName));
            }

            // Copy the related pictures.
            for (const QString &img : images) {
                const QString orig = QFileInfo(img).fileName();
                const QString newImgName = stem + QLatin1Char('_') + orig;
                const QString dst = destAbs + QLatin1Char('/') + newImgName;
                const QFileInfo dstImgInfo(dst);
                if (opts.overwrite && (dstImgInfo.exists() || dstImgInfo.isSymLink())) {
                    res.messages.append(QStringLiteral("覆盖已存在的文件: %1").arg(newImgName));
                    QFile::remove(dst);
                }
                if (QFile::copy(img, dst)) {
                    res.messages.append(QStringLiteral("复制图片: %1").arg(newImgName));
                } else {
                    res.messages.append(QStringLiteral("复制失败: %1").arg(newImgName));
                    anyError = true;
                }
            }
        }
    }

    // ── Rewrite \includegraphics references in the .tex files ──────────
    // Only references pointing into the link directory are rewritten: the
    // basename must be one of the successfully copied link files and the
    // directory must resolve to the link directory (the reference may reach
    // it through a symlinked path — canonical comparison covers that).
    for (const QString &texPath : res.texFiles) {
        QFile file(texPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            res.messages.append(QStringLiteral("无法读取: %1").arg(texPath));
            anyError = true;
            continue;
        }
        const QString content = QString::fromUtf8(file.readAll());
        file.close();

        const QString texDir = QFileInfo(texPath).absolutePath();
        int replacements = 0;
        const QString newContent = rewriteIncludes(
            content,
            [&](const QString &rawPath) -> QString {
                QString p = expandHome(rawPath);
                const QFileInfo fi(p);
                const QString abs = QDir::cleanPath(
                    fi.isAbsolute() ? p : texDir + QLatin1Char('/') + p);
                const QString refName = QFileInfo(abs).fileName();
                if (!newNameOf.contains(refName))
                    return QString();
                const QString refDir = QFileInfo(abs).absolutePath();
                if (refDir != linkDirAbs) {
                    const QString canonRefDir = QFileInfo(refDir).canonicalFilePath();
                    const QString canonLinkDir = QFileInfo(linkDirAbs).canonicalFilePath();
                    if (canonRefDir.isEmpty() || canonRefDir != canonLinkDir)
                        return QString();
                }
                return destConfigured + QLatin1Char('/') + newNameOf.value(refName);
            },
            &replacements);

        if (replacements > 0) {
            QFile out(texPath);
            if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
                res.messages.append(QStringLiteral("无法写入: %1").arg(texPath));
                anyError = true;
                continue;
            }
            out.write(newContent.toUtf8());
            out.close();
            res.messages.append(QStringLiteral("已更新引用: %1 (%2 处)").arg(texPath).arg(replacements));
            res.referenceReplacements += replacements;
        }
    }

    res.renamed = newNameOf;
    res.ok = !anyError;
    return res;
}

ProjectPackager::CliOutcome ProjectPackager::runPack(const QStringList &args)
{
    CliOutcome outcome;

    PackOptions opts;
    QString error, usage;
    if (!parsePackArgs(args, opts, error, usage)) {
        outcome.exitCode = 2;
        outcome.stderrLines << error;
        outcome.stderrLines << usage.split(QLatin1Char('\n'));
        return outcome;
    }

    if (opts.showHelp) {
        outcome.exitCode = 0;
        outcome.stdoutLines = usage.split(QLatin1Char('\n'));
        return outcome;
    }

    const PackResult res = pack(opts);
    outcome.stdoutLines = res.messages;
    if (!res.ok) {
        outcome.exitCode = 1;
        if (!res.errorMessage.isEmpty())
            outcome.stderrLines << QStringLiteral("错误: ") + res.errorMessage;
    }
    return outcome;
}

int ProjectPackager::runCli(const QStringList &args)
{
    int dummyArgc = 0;
    QCoreApplication app(dummyArgc, nullptr);
    QCoreApplication::setOrganizationName(QStringLiteral("HiTikZ"));
    QCoreApplication::setApplicationName(QStringLiteral("TikzManager"));

#ifdef RESOURCE_DIR
    // Make the bundled templates available so snippets selecting a template
    // are wrapped with it (the GUI does the same at startup).
    ensureTemplatesCopied(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                          + QStringLiteral("/templates/"));
#endif

    const CliOutcome outcome = runPack(args);

    QTextStream out(stdout);
    for (const QString &line : outcome.stdoutLines)
        out << line << Qt::endl;
    QTextStream err(stderr);
    for (const QString &line : outcome.stderrLines)
        err << line << Qt::endl;

    return outcome.exitCode;
}
