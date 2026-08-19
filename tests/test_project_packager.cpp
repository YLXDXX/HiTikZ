// Tests for the "hitikz pack" command-line feature (ProjectPackager):
//  - argument parsing (defaults, all options, errors, --help)
//  - tex argument expansion (file / directory / * and ? wildcards)
//  - link file collection (numeric sort, non-numbered files ignored)
//  - number formatting for the 01 / 001 / 0001 name formats
//  - \includegraphics rewriting (options preserved, comments untouched,
//    escaped % not a comment, multiple occurrences, only paths change)
//  - numbering with gap filling + skip messages when not overwriting
//  - sequential numbering + prominent overwrite messages when overwriting
//  - the three name formats end to end
//  - on-demand copying: only referenced pictures are copied (unreferenced
//    link files stay untouched), extensionless references (0001) resolved
//  - one image referenced by several .tex files is updated everywhere
//  - source copying (full .tex + related pictures with stem prefixes)
//  - source lookup via symlink target and via meta.json linkedPdf fallback
//  - error paths (missing files/dirs, dangling links, missing referenced
//    pictures, nothing referenced, usage errors)
//  - default link directory read from the program settings
#include "project_packager.h"
#include "link_manager.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>

static int g_failed = 0;

#define CHECK(expr, msg) \
    do { \
        if (!(expr)) { \
            qDebug() << "FAIL:" << msg; \
            g_failed++; \
        } \
    } while (0)

static bool writeFile(const QString &path, const QByteArray &content)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    f.write(content);
    f.close();
    return true;
}

static QByteArray readFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();
    const QByteArray data = f.readAll();
    f.close();
    return data;
}

static bool fileExists(const QString &path)
{
    const QFileInfo info(path);
    return info.exists() || info.isSymLink();
}

static QString joinLines(const QStringList &lines)
{
    return lines.join(QLatin1Char('\n'));
}

// ── 1. Number formatting ────────────────────────────────────────────────
static void testFormatNumber()
{
    CHECK(ProjectPackager::formatNumber(1, 2) == QStringLiteral("01"), "width 2 -> 01");
    CHECK(ProjectPackager::formatNumber(9, 2) == QStringLiteral("09"), "width 2 -> 09");
    CHECK(ProjectPackager::formatNumber(12, 2) == QStringLiteral("12"), "width 2 -> 12");
    CHECK(ProjectPackager::formatNumber(100, 2) == QStringLiteral("100"), "number grows past width 2");
    CHECK(ProjectPackager::formatNumber(1, 3) == QStringLiteral("001"), "width 3 -> 001");
    CHECK(ProjectPackager::formatNumber(1000, 3) == QStringLiteral("1000"), "number grows past width 3");
    CHECK(ProjectPackager::formatNumber(1, 4) == QStringLiteral("0001"), "width 4 -> 0001");
    CHECK(ProjectPackager::formatNumber(0, 2) == QStringLiteral("01"), "number 0 -> 01");
    CHECK(ProjectPackager::formatNumber(-5, 2) == QStringLiteral("01"), "negative -> 01");
    qDebug() << "PASS: Test 1 - number formatting (01/001/0001)";
}

// ── 2. Tex argument expansion ───────────────────────────────────────────
static void testExpandTexArgument()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    CHECK(writeFile(tmp.path() + "/a.tex", "a"), "a.tex created");
    CHECK(writeFile(tmp.path() + "/b.tex", "b"), "b.tex created");
    CHECK(writeFile(tmp.path() + "/notes.txt", "n"), "notes.txt created");
    QDir().mkdir(tmp.path() + "/sub");
    CHECK(writeFile(tmp.path() + "/sub/c.tex", "c"), "sub/c.tex created");

    // Single file.
    const QStringList single = ProjectPackager::expandTexArgument(tmp.path() + "/a.tex");
    CHECK(single == QStringList() << tmp.path() + "/a.tex", "single file passthrough");

    // Directory: all top-level .tex files, sorted by name.
    const QStringList dirFiles = ProjectPackager::expandTexArgument(tmp.path());
    CHECK(dirFiles == QStringList() << tmp.path() + "/a.tex" << tmp.path() + "/b.tex",
          "directory yields sorted top-level .tex files");

    // Wildcard *.
    const QStringList glob = ProjectPackager::expandTexArgument(tmp.path() + "/*.tex");
    CHECK(glob == QStringList() << tmp.path() + "/a.tex" << tmp.path() + "/b.tex",
          "*.tex glob matches top-level files");

    // Wildcard ? (single character names).
    const QStringList question = ProjectPackager::expandTexArgument(tmp.path() + "/?.tex");
    CHECK(question == QStringList() << tmp.path() + "/a.tex" << tmp.path() + "/b.tex",
          "?.tex glob matches single-character names");

    // Wildcard matching nothing.
    CHECK(ProjectPackager::expandTexArgument(tmp.path() + "/nope*.tex").isEmpty(),
          "wildcard with no matches yields empty list");
    qDebug() << "PASS: Test 2 - tex argument expansion";
}

// ── 3. Link file collection ─────────────────────────────────────────────
static void testCollectLinkFiles()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    CHECK(writeFile(tmp.path() + "/0003.pdf", "3"), "0003.pdf created");
    CHECK(writeFile(tmp.path() + "/0001.pdf", "1"), "0001.pdf created");
    CHECK(writeFile(tmp.path() + "/0010.pdf", "10"), "0010.pdf created");
    CHECK(writeFile(tmp.path() + "/0002.pdf", "2"), "0002.pdf created");
    CHECK(writeFile(tmp.path() + "/readme.txt", "x"), "readme.txt created");
    CHECK(writeFile(tmp.path() + "/notes.png", "x"), "notes.png created");
    CHECK(writeFile(tmp.path() + "/12abc.pdf", "x"), "non-numeric pdf created");
    CHECK(QFile::link(tmp.path() + "/missing.pdf", tmp.path() + "/0004.pdf"),
          "dangling symlink 0004.pdf created");

    const QStringList names = ProjectPackager::collectLinkFiles(tmp.path());
    CHECK(names == QStringList() << QStringLiteral("0001.pdf") << QStringLiteral("0002.pdf")
                                 << QStringLiteral("0003.pdf") << QStringLiteral("0004.pdf")
                                 << QStringLiteral("0010.pdf"),
          "numbered pdfs collected in numeric order, dangling symlink included");

    QTemporaryDir empty;
    CHECK(ProjectPackager::collectLinkFiles(empty.path()).isEmpty(),
          "empty dir yields empty list");
    CHECK(ProjectPackager::collectLinkFiles(tmp.path() + "/missing").isEmpty(),
          "missing dir yields empty list");
    qDebug() << "PASS: Test 3 - link file collection";
}

// ── 4. Argument parsing ─────────────────────────────────────────────────
static void testParsePackArgs()
{
    ProjectPackager::PackOptions opts;
    QString error, usage;

    // Defaults.
    CHECK(ProjectPackager::parsePackArgs(QStringList() << QStringLiteral("doc.tex")
                                                       << QStringLiteral("pics"),
                                         opts, error, usage),
          "minimal args parse");
    CHECK(opts.texArg == QStringLiteral("doc.tex"), "texArg set");
    CHECK(opts.destDir == QStringLiteral("pics"), "destDir set");
    CHECK(opts.nameWidth == 3, "default name format 001");
    CHECK(!opts.overwrite, "default no overwrite");
    CHECK(!opts.copySources, "default no source copying");
    CHECK(opts.linkDir.isEmpty(), "default link dir from settings");
    CHECK(!opts.showHelp, "no help requested");

    // All options, spaced and "=" forms.
    CHECK(ProjectPackager::parsePackArgs(
              QStringList() << QStringLiteral("--link-dir") << QStringLiteral("~/Pic")
                            << QStringLiteral("--name-format") << QStringLiteral("01")
                            << QStringLiteral("--overwrite") << QStringLiteral("--copy-sources")
                            << QStringLiteral("a.tex") << QStringLiteral("out/"),
              opts, error, usage),
          "full options parse");
    CHECK(opts.linkDir == QStringLiteral("~/Pic"), "link dir kept verbatim");
    CHECK(opts.nameWidth == 2, "01 -> width 2");
    CHECK(opts.overwrite, "overwrite set");
    CHECK(opts.copySources, "copy-sources set");
    CHECK(opts.destDir == QStringLiteral("out"), "trailing slash removed from dest");

    CHECK(ProjectPackager::parsePackArgs(
              QStringList() << QStringLiteral("--link-dir=/x") << QStringLiteral("--name-format=0001")
                            << QStringLiteral("b.tex") << QStringLiteral("o"),
              opts, error, usage),
          "equals-form options parse");
    CHECK(opts.linkDir == QStringLiteral("/x"), "--link-dir= value");
    CHECK(opts.nameWidth == 4, "0001 -> width 4");

    CHECK(ProjectPackager::parsePackArgs(
              QStringList() << QStringLiteral("--name-format") << QStringLiteral("001")
                            << QStringLiteral("b.tex") << QStringLiteral("o"),
              opts, error, usage),
          "001 parses");
    CHECK(opts.nameWidth == 3, "001 -> width 3");

    // --help.
    CHECK(ProjectPackager::parsePackArgs(QStringList() << QStringLiteral("--help"), opts, error, usage),
          "--help parses");
    CHECK(opts.showHelp, "showHelp set");

    // Errors.
    CHECK(!ProjectPackager::parsePackArgs(QStringList() << QStringLiteral("--bogus")
                                                        << QStringLiteral("a.tex")
                                                        << QStringLiteral("o"),
                                          opts, error, usage),
          "unknown option rejected");
    CHECK(error.contains(QStringLiteral("未知选项")), "unknown option message");

    CHECK(!ProjectPackager::parsePackArgs(QStringList() << QStringLiteral("--link-dir"),
                                          opts, error, usage),
          "--link-dir without value rejected");
    CHECK(error.contains(QStringLiteral("--link-dir")), "missing value message");

    CHECK(!ProjectPackager::parsePackArgs(
              QStringList() << QStringLiteral("--name-format") << QStringLiteral("05")
                            << QStringLiteral("a.tex") << QStringLiteral("o"),
              opts, error, usage),
          "invalid name format rejected");
    CHECK(error.contains(QStringLiteral("无效的命名格式")), "invalid format message");

    CHECK(!ProjectPackager::parsePackArgs(QStringList() << QStringLiteral("a.tex"), opts, error, usage),
          "single positional rejected");
    CHECK(error.contains(QStringLiteral("两个位置参数")), "positional count message");

    CHECK(!ProjectPackager::parsePackArgs(
              QStringList() << QStringLiteral("a.tex") << QStringLiteral("b") << QStringLiteral("c"),
              opts, error, usage),
          "three positionals rejected");

    CHECK(!ProjectPackager::parsePackArgs(
              QStringList() << QStringLiteral("a.tex") << QString(), opts, error, usage),
          "empty dest dir rejected");
    CHECK(error.contains(QStringLiteral("目标目录")), "empty dest message");

    CHECK(!ProjectPackager::parsePackArgs(
              QStringList() << QString() << QStringLiteral("o"), opts, error, usage),
          "empty tex arg rejected");

    CHECK(usage.contains(QStringLiteral("--name-format")), "usage text mentions options");
    qDebug() << "PASS: Test 4 - argument parsing";
}

// ── 5. includegraphics rewriting ────────────────────────────────────────
static void testRewriteIncludes()
{
    const QString content = QStringLiteral(
        "\\includegraphics{img1.pdf}\n"
        "\\includegraphics[width=0.5\\textwidth]{img2.pdf}\n"
        "\\includegraphics*{img3.pdf}\n"
        "\\includegraphics [trim=1 2 3 4]{img5.pdf}\n"
        "% commented \\includegraphics{img4.pdf}\n"
        "text \\% \\includegraphics{img6.pdf}\n"
        "\\includegraphics{img2.pdf}\n"
        "\\includegraphics{keep.pdf}\n"
        "\\includegraphics{~/PicTikZ/0001.pdf}\n");

    int count = -1;
    const QString out = ProjectPackager::rewriteIncludes(
        content,
        [](const QString &rawPath) -> QString {
            if (rawPath == QStringLiteral("img1.pdf"))
                return QStringLiteral("new1.pdf");
            if (rawPath == QStringLiteral("img2.pdf"))
                return QStringLiteral("new2.pdf");
            if (rawPath == QStringLiteral("img3.pdf"))
                return QStringLiteral("new3.pdf");
            if (rawPath == QStringLiteral("img5.pdf"))
                return QStringLiteral("new5.pdf");
            if (rawPath == QStringLiteral("img6.pdf"))
                return QStringLiteral("new6.pdf");
            return QString();
        },
        &count);

    CHECK(count == 6, "six occurrences rewritten");
    CHECK(out.contains(QStringLiteral("\\includegraphics{new1.pdf}")), "plain path rewritten");
    CHECK(out.contains(QStringLiteral("\\includegraphics[width=0.5\\textwidth]{new2.pdf}")),
          "options preserved");
    CHECK(out.contains(QStringLiteral("\\includegraphics*{new3.pdf}")), "star variant handled");
    CHECK(out.contains(QStringLiteral("\\includegraphics [trim=1 2 3 4]{new5.pdf}")),
          "space before options handled");
    CHECK(out.contains(QStringLiteral("% commented \\includegraphics{img4.pdf}")),
          "commented occurrence untouched");
    CHECK(out.contains(QStringLiteral("text \\% \\includegraphics{new6.pdf}")),
          "escaped percent is not a comment");
    CHECK(out.count(QStringLiteral("new2.pdf")) == 2,
          "all occurrences of one image rewritten");
    CHECK(out.count(QStringLiteral("\\includegraphics[width=0.5\\textwidth]{new2.pdf}")) == 1,
          "options form rewritten");
    CHECK(out.count(QStringLiteral("\\includegraphics{new2.pdf}")) == 1,
          "plain form rewritten");
    CHECK(out.contains(QStringLiteral("\\includegraphics{keep.pdf}")),
          "unmatched path untouched");
    CHECK(!out.contains(QStringLiteral("img1.pdf")), "no stale old name left");
    CHECK(!out.contains(QStringLiteral("img2.pdf}")), "no stale old name left (img2)");

    // Empty-content edge case.
    int zeroCount = -1;
    CHECK(ProjectPackager::rewriteIncludes(QString(), [](const QString &) { return QStringLiteral("x"); },
                                           &zeroCount)
              .isEmpty(),
          "empty content stays empty");
    CHECK(zeroCount == 0, "empty content rewrites nothing");
    qDebug() << "PASS: Test 5 - includegraphics rewriting";
}

// ── Helper: link directory inside the home directory (for ~ references) ─
struct HomeLinkDir {
    QTemporaryDir dir;
    QString linkDir; // absolute path
    QString refDir;  // "~" form to use in .tex references
    HomeLinkDir()
        : dir(QDir::homePath() + QStringLiteral("/.hitikz-pack-test-XXXXXX"))
    {
        linkDir = dir.path();
        refDir = QStringLiteral("~/") + QFileInfo(dir.path()).fileName();
    }
};

// ── 6. No-overwrite numbering with gap filling ──────────────────────────
static void testPackNoOverwriteGapFilling()
{
    HomeLinkDir home;
    CHECK(writeFile(home.linkDir + "/0001.pdf", QByteArray("content-a")), "link 0001.pdf created");
    CHECK(writeFile(home.linkDir + "/0003.pdf", QByteArray("content-b")), "link 0003.pdf created");

    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString dest = tmp.path() + "/pics";
    QDir().mkdir(dest);
    CHECK(writeFile(dest + "/01.pdf", QByteArray("old-1")), "dest 01.pdf created");
    CHECK(writeFile(dest + "/03.pdf", QByteArray("old-3")), "dest 03.pdf created");

    const QString texPath = tmp.path() + "/doc.tex";
    const QString texContent = QStringLiteral(
        "\\documentclass{article}\n"
        "\\usepackage{graphicx}\n"
        "\\begin{document}\n"
        "\\includegraphics{%1/0001.pdf}\n"
        "\\includegraphics[width=2cm]{%1/0001.pdf}\n"
        "\\includegraphics{%1/0003.pdf}\n"
        "\\end{document}\n")
        .arg(home.refDir);
    CHECK(writeFile(texPath, texContent.toUtf8()), "doc.tex created");

    ProjectPackager::PackOptions opts;
    opts.texArg = texPath;
    opts.destDir = dest;
    opts.linkDir = home.linkDir;
    opts.nameWidth = 2;
    const ProjectPackager::PackResult res = ProjectPackager::pack(opts);

    CHECK(res.ok, "pack succeeds");
    CHECK(res.renamed.value(QStringLiteral("0001.pdf")) == QStringLiteral("02.pdf"),
          "0001.pdf -> 02.pdf (gap filled)");
    CHECK(res.renamed.value(QStringLiteral("0003.pdf")) == QStringLiteral("04.pdf"),
          "0003.pdf -> 04.pdf (gap filled)");
    CHECK(joinLines(res.messages).contains(QStringLiteral("跳过已存在的文件: 01.pdf")),
          "existing 01.pdf reported as skipped");
    CHECK(joinLines(res.messages).contains(QStringLiteral("跳过已存在的文件: 03.pdf")),
          "existing 03.pdf reported as skipped");
    CHECK(!joinLines(res.messages).contains(QStringLiteral("覆盖")), "no overwrite messages");

    CHECK(readFile(dest + "/02.pdf") == QByteArray("content-a"), "02.pdf has 0001 content");
    CHECK(readFile(dest + "/04.pdf") == QByteArray("content-b"), "04.pdf has 0003 content");
    CHECK(readFile(dest + "/01.pdf") == QByteArray("old-1"), "existing 01.pdf untouched");
    CHECK(readFile(dest + "/03.pdf") == QByteArray("old-3"), "existing 03.pdf untouched");

    const QString rewritten = QString::fromUtf8(readFile(texPath));
    CHECK(rewritten.contains(QStringLiteral("\\includegraphics{%1/02.pdf}").arg(dest)),
          "reference rewritten to new path");
    CHECK(rewritten.contains(QStringLiteral("\\includegraphics[width=2cm]{%1/02.pdf}").arg(dest)),
          "options preserved in rewritten reference");
    CHECK(rewritten.count(QStringLiteral("%1/02.pdf").arg(dest)) == 2,
          "both occurrences of 0001.pdf rewritten");
    CHECK(rewritten.contains(QStringLiteral("\\includegraphics{%1/04.pdf}").arg(dest)),
          "0003.pdf reference rewritten");
    CHECK(!rewritten.contains(QStringLiteral("0001.pdf")), "no stale 0001.pdf left");
    CHECK(!rewritten.contains(QStringLiteral("0003.pdf")), "no stale 0003.pdf left");
    CHECK(res.referenceReplacements == 3, "three references replaced");
    CHECK(joinLines(res.messages).contains(QStringLiteral("已更新引用: %1 (3 处)").arg(texPath)),
          "rewrite message shows the count");
    qDebug() << "PASS: Test 6 - no-overwrite gap filling";
}

// ── 7. Overwrite mode ───────────────────────────────────────────────────
static void testPackOverwrite()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString linkDir = tmp.path() + "/links";
    QDir().mkdir(linkDir);
    CHECK(writeFile(linkDir + "/0001.pdf", QByteArray("new-a")), "link 0001.pdf created");
    CHECK(writeFile(linkDir + "/0002.pdf", QByteArray("new-b")), "link 0002.pdf created");

    const QString dest = tmp.path() + "/pics";
    QDir().mkdir(dest);
    CHECK(writeFile(dest + "/01.pdf", QByteArray("old-a")), "dest 01.pdf created");
    CHECK(writeFile(dest + "/02.pdf", QByteArray("old-b")), "dest 02.pdf created");
    CHECK(writeFile(dest + "/05.pdf", QByteArray("keep-5")), "dest 05.pdf created");

    const QString texPath = tmp.path() + "/doc.tex";
    // Only 0001.pdf is referenced: 0002.pdf must stay untouched even with
    // --overwrite (on-demand copying).
    CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(linkDir).toUtf8()),
          "doc.tex created");

    ProjectPackager::PackOptions opts;
    opts.texArg = texPath;
    opts.destDir = dest;
    opts.linkDir = linkDir;
    opts.nameWidth = 2;
    opts.overwrite = true;
    const ProjectPackager::PackResult res = ProjectPackager::pack(opts);

    CHECK(res.ok, "pack succeeds");
    CHECK(res.renamed.value(QStringLiteral("0001.pdf")) == QStringLiteral("01.pdf"),
          "0001.pdf -> 01.pdf (sequential)");
    CHECK(!res.renamed.contains(QStringLiteral("0002.pdf")),
          "unreferenced 0002.pdf not copied");
    CHECK(joinLines(res.messages).contains(QStringLiteral("覆盖已存在的文件: 01.pdf")),
          "overwriting 01.pdf prominently reported");
    CHECK(!joinLines(res.messages).contains(QStringLiteral("覆盖已存在的文件: 02.pdf")),
          "unreferenced 02.pdf not overwritten");
    CHECK(!joinLines(res.messages).contains(QStringLiteral("0002.pdf ->")),
          "no copy message for unreferenced 0002.pdf");
    CHECK(!joinLines(res.messages).contains(QStringLiteral("跳过")), "no skip messages in overwrite mode");

    CHECK(readFile(dest + "/01.pdf") == QByteArray("new-a"), "01.pdf overwritten with new content");
    CHECK(readFile(dest + "/02.pdf") == QByteArray("old-b"), "unreferenced 02.pdf untouched");
    CHECK(readFile(dest + "/05.pdf") == QByteArray("keep-5"), "05.pdf untouched");

    const QString rewritten = QString::fromUtf8(readFile(texPath));
    CHECK(rewritten.contains(QStringLiteral("\\includegraphics{%1/01.pdf}").arg(dest)),
          "reference rewritten in overwrite mode");
    qDebug() << "PASS: Test 7 - overwrite mode";
}

// ── 8. Name formats 01 / 001 / 0001 ─────────────────────────────────────
static void testPackNameFormats()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString linkDir = tmp.path() + "/links";
    QDir().mkdir(linkDir);
    CHECK(writeFile(linkDir + "/0001.pdf", "1"), "link 0001.pdf created");
    CHECK(writeFile(linkDir + "/0002.pdf", "2"), "link 0002.pdf created");
    CHECK(writeFile(linkDir + "/0003.pdf", "3"), "link 0003.pdf created");

    const QString texPath = tmp.path() + "/doc.tex";
    // Only 0001.pdf is referenced; the other link files stay uncopied.
    CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(linkDir).toUtf8()),
          "doc.tex created");

    const struct {
        int width;
        const char *first;
    } cases[] = {
        { 2, "01.pdf" },
        { 3, "001.pdf" },
        { 4, "0001.pdf" },
    };
    for (const auto &c : cases) {
        const QString dest = tmp.path() + QStringLiteral("/out%1").arg(c.width);
        ProjectPackager::PackOptions opts;
        opts.texArg = texPath;
        opts.destDir = dest;
        opts.linkDir = linkDir;
        opts.nameWidth = c.width;
        const ProjectPackager::PackResult res = ProjectPackager::pack(opts);
        CHECK(res.ok, "pack succeeds for width");
        CHECK(res.renamed.value(QStringLiteral("0001.pdf")) == QString::fromLatin1(c.first),
              QStringLiteral("width %1 -> %2").arg(c.width).arg(QString::fromLatin1(c.first)));
        CHECK(res.renamed.size() == 1, "only the referenced picture is copied");
        CHECK(fileExists(dest + QStringLiteral("/") + QString::fromLatin1(c.first)),
              "referenced pdf file written");
        CHECK(!fileExists(dest + QStringLiteral("/") + QString::fromLatin1("0002.pdf")),
              "unreferenced 0002.pdf not written");
        CHECK(!fileExists(dest + QStringLiteral("/") + QString::fromLatin1("0003.pdf")),
              "unreferenced 0003.pdf not written");
        // The .tex reference must use the width of this run.
        const QString rewritten = QString::fromUtf8(readFile(texPath));
        CHECK(rewritten.contains(
                  QStringLiteral("\\includegraphics{%1/%2}").arg(dest, QString::fromLatin1(c.first))),
              "reference uses the configured name format");
        // Restore the tex file for the next width.
        CHECK(writeFile(texPath,
                        QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(linkDir).toUtf8()),
              "doc.tex restored");
    }
    qDebug() << "PASS: Test 8 - name formats 01/001/0001";
}

// ── 9. Multiple .tex files sharing one image ────────────────────────────
static void testPackMultipleTexFiles()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString linkDir = tmp.path() + "/links";
    QDir().mkdir(linkDir);
    CHECK(writeFile(linkDir + "/0001.pdf", QByteArray("content")), "link 0001.pdf created");

    const QString dest = tmp.path() + "/pics";
    const QString texA = tmp.path() + "/a.tex";
    const QString texB = tmp.path() + "/b.tex";
    CHECK(writeFile(texA, QStringLiteral("A \\includegraphics{%1/0001.pdf} A\n").arg(linkDir).toUtf8()),
          "a.tex created");
    CHECK(writeFile(texB, QStringLiteral("B \\includegraphics[scale=0.8]{%1/0001.pdf} B\n").arg(linkDir).toUtf8()),
          "b.tex created");
    CHECK(writeFile(tmp.path() + "/c.txt", "x"), "non-tex file present");

    ProjectPackager::PackOptions opts;
    opts.texArg = tmp.path(); // directory: processes a.tex and b.tex
    opts.destDir = dest;
    opts.linkDir = linkDir;
    const ProjectPackager::PackResult res = ProjectPackager::pack(opts);

    CHECK(res.ok, "pack succeeds");
    CHECK(res.texFiles.size() == 2, "two tex files processed");
    CHECK(joinLines(res.messages).contains(QStringLiteral("已更新引用: %1 (1 处)").arg(texA)),
          "a.tex rewrite reported");
    CHECK(joinLines(res.messages).contains(QStringLiteral("已更新引用: %1 (1 处)").arg(texB)),
          "b.tex rewrite reported");
    CHECK(QString::fromUtf8(readFile(texA)).contains(QStringLiteral("%1/001.pdf").arg(dest)),
          "a.tex rewritten");
    CHECK(QString::fromUtf8(readFile(texB)).contains(
              QStringLiteral("\\includegraphics[scale=0.8]{%1/001.pdf}").arg(dest)),
          "b.tex rewritten with options preserved");
    CHECK(res.referenceReplacements == 2, "two total replacements");
    qDebug() << "PASS: Test 9 - multiple tex files";
}

// ── 9b. On-demand copying: only referenced pictures are copied ──────────
static void testPackCopiesOnlyReferenced()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString linkDir = tmp.path() + "/links";
    QDir().mkdir(linkDir);
    CHECK(writeFile(linkDir + "/0001.pdf", QByteArray("content-a")), "link 0001.pdf created");
    CHECK(writeFile(linkDir + "/0002.pdf", QByteArray("content-b")), "link 0002.pdf created");
    CHECK(writeFile(linkDir + "/0003.pdf", QByteArray("content-c")), "link 0003.pdf created");

    const QString dest = tmp.path() + "/pics";
    const QString texPath = tmp.path() + "/doc.tex";
    const QString texContent = QStringLiteral(
        "\\includegraphics{%1/0001.pdf}\n"
        "\\includegraphics[width=2cm]{%1/0001.pdf}\n"
        "\\includegraphics{%1/0003.pdf}\n"
        "\\includegraphics{%2/local.png}\n")
        .arg(linkDir, tmp.path());
    CHECK(writeFile(texPath, texContent.toUtf8()), "doc.tex created");

    ProjectPackager::PackOptions opts;
    opts.texArg = texPath;
    opts.destDir = dest;
    opts.linkDir = linkDir;
    const ProjectPackager::PackResult res = ProjectPackager::pack(opts);

    CHECK(res.ok, "pack succeeds");
    CHECK(res.renamed.value(QStringLiteral("0001.pdf")) == QStringLiteral("001.pdf"),
          "referenced 0001.pdf -> 001.pdf");
    CHECK(res.renamed.value(QStringLiteral("0003.pdf")) == QStringLiteral("002.pdf"),
          "referenced 0003.pdf -> 002.pdf (numbering only counts copied pictures)");
    CHECK(!res.renamed.contains(QStringLiteral("0002.pdf")),
          "unreferenced 0002.pdf not copied");
    CHECK(res.renamed.size() == 2, "exactly two pictures copied");

    CHECK(readFile(dest + "/001.pdf") == QByteArray("content-a"), "001.pdf has 0001 content");
    CHECK(readFile(dest + "/002.pdf") == QByteArray("content-c"), "002.pdf has 0003 content");
    CHECK(!fileExists(dest + "/003.pdf"), "no extra pdf written");

    CHECK(joinLines(res.messages).contains(QStringLiteral("复制: 0001.pdf -> 001.pdf")),
          "copy of 0001.pdf reported");
    CHECK(joinLines(res.messages).contains(QStringLiteral("复制: 0003.pdf -> 002.pdf")),
          "copy of 0003.pdf reported");
    CHECK(!joinLines(res.messages).contains(QStringLiteral("0002.pdf")),
          "unreferenced 0002.pdf never mentioned");

    const QString rewritten = QString::fromUtf8(readFile(texPath));
    CHECK(rewritten.count(QStringLiteral("%1/001.pdf").arg(dest)) == 2,
          "both 0001.pdf references rewritten");
    CHECK(rewritten.contains(QStringLiteral("\\includegraphics[width=2cm]{%1/001.pdf}").arg(dest)),
          "options preserved");
    CHECK(rewritten.contains(QStringLiteral("\\includegraphics{%1/002.pdf}").arg(dest)),
          "0003.pdf reference rewritten");
    CHECK(rewritten.contains(QStringLiteral("\\includegraphics{%1/local.png}").arg(tmp.path())),
          "non-link reference untouched");
    CHECK(res.referenceReplacements == 3, "three references replaced");
    qDebug() << "PASS: Test 9b - on-demand copying of referenced pictures only";
}

// ── 9c. Extensionless references (0001 instead of 0001.pdf) ─────────────
static void testPackExtensionlessReference()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString linkDir = tmp.path() + "/links";
    QDir().mkdir(linkDir);
    CHECK(writeFile(linkDir + "/0001.pdf", QByteArray("content")), "link 0001.pdf created");

    const QString dest = tmp.path() + "/pics";
    const QString texPath = tmp.path() + "/doc.tex";
    CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001}\n").arg(linkDir).toUtf8()),
          "doc.tex with extensionless reference created");

    ProjectPackager::PackOptions opts;
    opts.texArg = texPath;
    opts.destDir = dest;
    opts.linkDir = linkDir;
    const ProjectPackager::PackResult res = ProjectPackager::pack(opts);

    CHECK(res.ok, "pack succeeds");
    CHECK(res.renamed.value(QStringLiteral("0001.pdf")) == QStringLiteral("001.pdf"),
          "extensionless reference resolved to 0001.pdf");
    CHECK(readFile(dest + "/001.pdf") == QByteArray("content"), "pdf copied");
    const QString rewritten = QString::fromUtf8(readFile(texPath));
    CHECK(rewritten.contains(QStringLiteral("\\includegraphics{%1/001.pdf}").arg(dest)),
          "extensionless reference rewritten with .pdf extension");
    CHECK(!rewritten.contains(QStringLiteral("0001")), "no stale reference left");

    // An extensionless reference whose pdf is missing is left alone.
    const QString texPath2 = tmp.path() + "/doc2.tex";
    CHECK(writeFile(texPath2, QStringLiteral("\\includegraphics{%1/9999}\n").arg(linkDir).toUtf8()),
          "doc2.tex created");
    ProjectPackager::PackOptions opts2;
    opts2.texArg = texPath2;
    opts2.destDir = tmp.path() + "/pics2";
    opts2.linkDir = linkDir;
    const ProjectPackager::PackResult res2 = ProjectPackager::pack(opts2);
    CHECK(res2.ok, "missing extensionless pdf is not an error");
    CHECK(QString::fromUtf8(readFile(texPath2)).contains(QStringLiteral("%1/9999").arg(linkDir)),
          "unresolvable extensionless reference untouched");
    CHECK(!fileExists(tmp.path() + "/pics2/001.pdf"), "nothing copied");
    qDebug() << "PASS: Test 9c - extensionless references";
}

// ── 10. Source copying via symlink target ───────────────────────────────
static void testPackCopySources()
{
    SnippetManager mgr;
    Snippet s;
    s.id = mgr.createSnippet(QStringLiteral("测试片段"), QStringLiteral("测试"));
    CHECK(!s.id.isEmpty(), "snippet created");
    s.name = QStringLiteral("测试片段");
    s.code = QStringLiteral("\\begin{tikzpicture}\n"
                            "\\node at (0,0) {\\includegraphics{a.png}};\n"
                            "\\end{tikzpicture}\n");
    CHECK(mgr.saveSnippet(s), "snippet saved");

    // A related picture (dummy content; only the file copy matters here).
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString dummyImage = tmp.path() + "/dummy.png";
    CHECK(writeFile(dummyImage, QByteArray("png-bytes")), "dummy image created");
    const QString imageName = mgr.addImageToSnippet(s.id, dummyImage);
    CHECK(imageName == QStringLiteral("a.png"), "image added as a.png");

    // preview.pdf in the snippet directory, linked as 0001.pdf.
    const QString preview = mgr.getBasePath() + s.id + "/preview.pdf";
    CHECK(writeFile(preview, QByteArray("pdf-bytes")), "preview.pdf created");
    const QString linkDir = tmp.path() + "/links";
    LinkManager links(linkDir);
    CHECK(links.createLink(preview, QStringLiteral("0001.pdf")), "symlink 0001.pdf created");

    const QString texPath = tmp.path() + "/doc.tex";
    CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(linkDir).toUtf8()),
          "doc.tex created");

    ProjectPackager::PackOptions opts;
    opts.texArg = texPath;
    opts.destDir = tmp.path() + "/pics";
    opts.linkDir = linkDir;
    opts.copySources = true;
    const ProjectPackager::PackResult res = ProjectPackager::pack(opts);

    CHECK(res.ok, "pack succeeds");
    CHECK(res.renamed.value(QStringLiteral("0001.pdf")) == QStringLiteral("001.pdf"),
          "pdf numbered 001");

    const QString dest = tmp.path() + "/pics";
    CHECK(readFile(dest + "/001.pdf") == QByteArray("pdf-bytes"), "001.pdf copied");
    CHECK(fileExists(dest + "/001.tex"), "001.tex written");
    CHECK(fileExists(dest + "/001_a.png"), "001_a.png written");
    CHECK(readFile(dest + "/001_a.png") == QByteArray("png-bytes"), "image content copied");
    CHECK(joinLines(res.messages).contains(QStringLiteral("复制源码: 001.tex")),
          "source copy reported");
    CHECK(joinLines(res.messages).contains(QStringLiteral("复制图片: 001_a.png")),
          "picture copy reported");

    const QString fullTex = QString::fromUtf8(readFile(dest + "/001.tex"));
    CHECK(fullTex.startsWith(QStringLiteral("%% name: 测试片段")), "metadata header present");
    CHECK(fullTex.contains(QStringLiteral("\\includegraphics{001_a.png}")),
          "full document references the renamed picture");
    CHECK(!fullTex.contains(QStringLiteral("\\includegraphics{a.png}")),
          "no stale picture reference left in the full document");
    CHECK(fullTex.contains(QStringLiteral("\\begin{document}")), "full document wrapped");

    CHECK(QString::fromUtf8(readFile(texPath)).contains(QStringLiteral("%1/001.pdf").arg(dest)),
          "document reference rewritten");
    qDebug() << "PASS: Test 10 - source copying via symlink target";
}

// ── 11. Source copying via meta.json linkedPdf fallback ─────────────────
static void testPackCopySourcesFallback()
{
    SnippetManager mgr;
    const QString id = mgr.createSnippet(QStringLiteral("回退片段"), QStringLiteral("测试"));
    CHECK(!id.isEmpty(), "snippet created");
    Snippet s = mgr.loadSnippet(id);
    s.linkedPdf = QStringLiteral("0001.pdf");
    s.code = QStringLiteral("\\begin{tikzpicture}\n\\draw (0,0) -- (1,1);\n\\end{tikzpicture}\n");
    CHECK(mgr.saveSnippet(s), "snippet saved with linkedPdf");

    const QString preview = mgr.getBasePath() + id + "/preview.pdf";
    CHECK(writeFile(preview, QByteArray("pdf-bytes")), "preview.pdf created");

    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString linkDir = tmp.path() + "/links";
    QDir().mkdir(linkDir);
    // A plain file (no symlink) so the meta.json scan is the only route.
    CHECK(QFile::copy(preview, linkDir + "/0001.pdf"), "plain copy link created");

    const QString texPath = tmp.path() + "/doc.tex";
    CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(linkDir).toUtf8()),
          "doc.tex created");

    ProjectPackager::PackOptions opts;
    opts.texArg = texPath;
    opts.destDir = tmp.path() + "/pics";
    opts.linkDir = linkDir;
    opts.copySources = true;
    const ProjectPackager::PackResult res = ProjectPackager::pack(opts);

    CHECK(res.ok, "pack succeeds");
    const QString dest = tmp.path() + "/pics";
    CHECK(fileExists(dest + "/001.tex"), "001.tex written via fallback");
    CHECK(QString::fromUtf8(readFile(dest + "/001.tex")).startsWith(QStringLiteral("%% name: 回退片段")),
          "fallback found the right snippet");
    qDebug() << "PASS: Test 11 - source lookup via meta.json fallback";
}

// ── 12. Error paths ─────────────────────────────────────────────────────
static void testPackErrors()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString linkDir = tmp.path() + "/links";
    QDir().mkdir(linkDir);
    CHECK(writeFile(linkDir + "/0001.pdf", QByteArray("content")), "link 0001.pdf created");

    // Missing tex file.
    {
        ProjectPackager::PackOptions opts;
        opts.texArg = tmp.path() + "/missing.tex";
        opts.destDir = tmp.path() + "/pics";
        opts.linkDir = linkDir;
        const ProjectPackager::PackResult res = ProjectPackager::pack(opts);
        CHECK(!res.ok, "missing tex file fails");
        CHECK(res.errorMessage.contains(QStringLiteral("TeX 文件不存在")), "missing tex message");
    }

    // Wildcard matching nothing.
    {
        ProjectPackager::PackOptions opts;
        opts.texArg = tmp.path() + "/nope*.tex";
        opts.destDir = tmp.path() + "/pics";
        opts.linkDir = linkDir;
        const ProjectPackager::PackResult res = ProjectPackager::pack(opts);
        CHECK(!res.ok, "empty glob fails");
        CHECK(res.errorMessage.contains(QStringLiteral("没有找到 TeX 文件")), "empty glob message");
    }

    // Missing link directory.
    {
        ProjectPackager::PackOptions opts;
        opts.texArg = tmp.path();
        opts.destDir = tmp.path() + "/pics";
        opts.linkDir = tmp.path() + "/no-links";
        const ProjectPackager::PackResult res = ProjectPackager::pack(opts);
        CHECK(!res.ok, "missing link dir fails");
        CHECK(res.errorMessage.contains(QStringLiteral("链接图片目录不存在")), "missing link dir message");
    }

    // Dangling link: copy fails and the reference stays untouched.
    {
        const QString danglingDir = tmp.path() + "/dangling";
        QDir().mkdir(danglingDir);
        CHECK(QFile::link(tmp.path() + "/missing-target.pdf", danglingDir + "/0001.pdf"),
              "dangling symlink created");
        const QString texPath = tmp.path() + "/dangling.tex";
        CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(danglingDir).toUtf8()),
              "dangling.tex created");
        ProjectPackager::PackOptions opts;
        opts.texArg = texPath;
        opts.destDir = tmp.path() + "/pics";
        opts.linkDir = danglingDir;
        const ProjectPackager::PackResult res = ProjectPackager::pack(opts);
        CHECK(!res.ok, "dangling link fails the run");
        CHECK(joinLines(res.messages).contains(QStringLiteral("复制失败: 0001.pdf")),
              "copy failure reported");
        CHECK(res.renamed.isEmpty(), "no rename recorded for failed copy");
        CHECK(QString::fromUtf8(readFile(texPath)).contains(QStringLiteral("%1/0001.pdf").arg(danglingDir)),
              "reference to failed copy untouched");
    }

    // Referenced link picture is missing from the link directory.
    {
        const QString emptyDir = tmp.path() + "/empty-links";
        QDir().mkdir(emptyDir);
        CHECK(writeFile(emptyDir + "/readme.txt", "x"), "non-pdf file in link dir");
        const QString texPath = tmp.path() + "/empty.tex";
        CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(emptyDir).toUtf8()),
              "empty.tex created");
        ProjectPackager::PackOptions opts;
        opts.texArg = texPath;
        opts.destDir = tmp.path() + "/pics";
        opts.linkDir = emptyDir;
        const ProjectPackager::PackResult res = ProjectPackager::pack(opts);
        CHECK(!res.ok, "missing referenced picture fails the run");
        CHECK(joinLines(res.messages).contains(QStringLiteral("引用的链接图片不存在: 0001.pdf")),
              "missing reference reported");
        CHECK(QString::fromUtf8(readFile(texPath)).contains(QStringLiteral("%1/0001.pdf").arg(emptyDir)),
              "reference to missing picture untouched");
    }

    // Documents reference nothing from the link directory: success, no copy.
    {
        const QString texPath = tmp.path() + "/norefs.tex";
        CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/local.png}\n").arg(tmp.path()).toUtf8()),
              "norefs.tex created");
        ProjectPackager::PackOptions opts;
        opts.texArg = texPath;
        opts.destDir = tmp.path() + "/pics-norefs";
        opts.linkDir = linkDir;
        const ProjectPackager::PackResult res = ProjectPackager::pack(opts);
        CHECK(res.ok, "no referenced pictures is not a failure");
        CHECK(joinLines(res.messages).contains(QStringLiteral("没有引用链接目录中的图片")),
              "info message about nothing referenced");
        CHECK(res.renamed.isEmpty(), "nothing copied");
        CHECK(QString::fromUtf8(readFile(texPath)).contains(QStringLiteral("%1/local.png").arg(tmp.path())),
              "tex file untouched when nothing referenced");
    }
    qDebug() << "PASS: Test 12 - error paths";
}

// ── 13. Default link dir from program settings ──────────────────────────
static void testPackUsesSettingLinkDir()
{
    QSettings settings(QStringLiteral("HiTikZ"), QStringLiteral("TikzManager"));
    const bool hadKey = settings.contains(QStringLiteral("link/dir"));
    const QVariant oldValue = settings.value(QStringLiteral("link/dir"));

    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString linkDir = tmp.path() + "/links";
    QDir().mkdir(linkDir);
    CHECK(writeFile(linkDir + "/0001.pdf", QByteArray("content")), "link 0001.pdf created");
    settings.setValue(QStringLiteral("link/dir"), linkDir);

    const QString texPath = tmp.path() + "/doc.tex";
    CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(linkDir).toUtf8()),
          "doc.tex created");

    ProjectPackager::PackOptions opts;
    opts.texArg = texPath;
    opts.destDir = tmp.path() + "/pics";
    // opts.linkDir left empty -> settings are consulted.
    const ProjectPackager::PackResult res = ProjectPackager::pack(opts);

    CHECK(res.ok, "pack succeeds with settings-provided link dir");
    CHECK(res.renamed.value(QStringLiteral("0001.pdf")) == QStringLiteral("001.pdf"),
          "pdf copied from settings dir");
    CHECK(fileExists(tmp.path() + "/pics/001.pdf"), "destination file exists");
    CHECK(joinLines(res.messages).contains(QStringLiteral("链接图片目录: %1").arg(linkDir)),
          "settings dir shown in messages");

    if (hadKey)
        settings.setValue(QStringLiteral("link/dir"), oldValue);
    else
        settings.remove(QStringLiteral("link/dir"));
    qDebug() << "PASS: Test 13 - default link dir from settings";
}

// ── 14. CLI outcome (exit codes, stdout/stderr) ─────────────────────────
static void testRunPack()
{
    // Usage error -> exit 2.
    {
        const ProjectPackager::CliOutcome outcome = ProjectPackager::runPack(QStringList());
        CHECK(outcome.exitCode == 2, "no arguments -> usage error");
        CHECK(!outcome.stderrLines.isEmpty(), "usage error printed to stderr");
        CHECK(joinLines(outcome.stderrLines).contains(QStringLiteral("用法")), "stderr carries usage");
    }
    {
        const ProjectPackager::CliOutcome outcome =
            ProjectPackager::runPack(QStringList() << QStringLiteral("only-one.tex"));
        CHECK(outcome.exitCode == 2, "single positional -> usage error");
    }
    {
        const ProjectPackager::CliOutcome outcome =
            ProjectPackager::runPack(QStringList() << QStringLiteral("--nope")
                                                   << QStringLiteral("a.tex") << QStringLiteral("o"));
        CHECK(outcome.exitCode == 2, "unknown option -> usage error");
    }

    // --help -> exit 0 with usage on stdout.
    {
        const ProjectPackager::CliOutcome outcome =
            ProjectPackager::runPack(QStringList() << QStringLiteral("--help"));
        CHECK(outcome.exitCode == 0, "--help exits 0");
        CHECK(joinLines(outcome.stdoutLines).contains(QStringLiteral("用法")), "help on stdout");
    }

    // Runtime error -> exit 1.
    {
        QTemporaryDir tmp;
        CHECK(tmp.isValid(), "temp dir valid");
        const ProjectPackager::CliOutcome outcome = ProjectPackager::runPack(
            QStringList() << tmp.path() + "/missing.tex" << tmp.path() + "/pics");
        CHECK(outcome.exitCode == 1, "missing tex file -> exit 1");
        CHECK(joinLines(outcome.stderrLines).contains(QStringLiteral("错误")), "error on stderr");
    }

    // Success -> exit 0 with copy messages on stdout.
    {
        QTemporaryDir tmp;
        CHECK(tmp.isValid(), "temp dir valid");
        const QString linkDir = tmp.path() + "/links";
        QDir().mkdir(linkDir);
        CHECK(writeFile(linkDir + "/0001.pdf", QByteArray("content")), "link 0001.pdf created");
        const QString texPath = tmp.path() + "/doc.tex";
        CHECK(writeFile(texPath, QStringLiteral("\\includegraphics{%1/0001.pdf}\n").arg(linkDir).toUtf8()),
              "doc.tex created");
        const ProjectPackager::CliOutcome outcome = ProjectPackager::runPack(
            QStringList() << QStringLiteral("--name-format") << QStringLiteral("01")
                          << texPath << tmp.path() + "/pics"
                          << QStringLiteral("--link-dir") << linkDir);
        CHECK(outcome.exitCode == 0, "successful run exits 0");
        CHECK(joinLines(outcome.stdoutLines).contains(QStringLiteral("复制: 0001.pdf -> 01.pdf")),
              "copy message on stdout");
        CHECK(outcome.stderrLines.isEmpty(), "no stderr output on success");
    }
    qDebug() << "PASS: Test 14 - CLI outcome";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("hitikz-test-packager");
    QCoreApplication::setOrganizationName("HiTikZ");
    QStandardPaths::setTestModeEnabled(true);

    // Clean leftovers from previous test runs so the meta.json fallback scan
    // in the source-copy tests cannot pick up stale snippets.
    {
        SnippetManager mgr;
        QDir(mgr.getBasePath()).removeRecursively();
        QDir(mgr.getPresetPath()).removeRecursively();
    }

    testFormatNumber();
    testExpandTexArgument();
    testCollectLinkFiles();
    testParsePackArgs();
    testRewriteIncludes();
    testPackNoOverwriteGapFilling();
    testPackOverwrite();
    testPackNameFormats();
    testPackMultipleTexFiles();
    testPackCopiesOnlyReferenced();
    testPackExtensionlessReference();
    testPackCopySources();
    testPackCopySourcesFallback();
    testPackErrors();
    testPackUsesSettingLinkDir();
    testRunPack();

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) FAILED!";
        return 1;
    }
    qDebug() << "\nAll tests passed!";
    return 0;
}
