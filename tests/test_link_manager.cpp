// Tests for the "复制链接" picture-link feature (LinkManager):
//  - default and custom link directories ("~" expansion, trailing slash)
//  - \includegraphics command generation
//  - sequence number allocation with gap filling (0001/0003/0004 -> 0002)
//  - non-sequence files and dangling symlinks keep their names occupied
//  - symlink creation (with copy fallback), replacement and removal
//  - entry vs. target existence (dangling link detection)
#include "link_manager.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
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

static void testDefaultDir()
{
    LinkManager links;
    CHECK(links.configuredDir() == QStringLiteral("~/PicTikZ"),
          "default configured dir is ~/PicTikZ");
    CHECK(links.dirPath() == QDir::homePath() + QStringLiteral("/PicTikZ"),
          "~ expanded to home directory");
    CHECK(links.includeCommand(QStringLiteral("0002.pdf"))
              == QStringLiteral("\\includegraphics{~/PicTikZ/0002.pdf}"),
          "include command uses the configured dir verbatim");
    CHECK(links.includeCommand(QStringLiteral("0002.pdf")).contains("\\includegraphics"),
          "include command contains \\includegraphics");
    qDebug() << "PASS: Test 1 - default directory and include command";
}

static void testCustomDirNormalization()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    const QString raw = tmp.path() + QStringLiteral("/sub/dir/");

    LinkManager links(raw);
    CHECK(links.configuredDir() == tmp.path() + QStringLiteral("/sub/dir"),
          "trailing slash removed from configured dir");
    CHECK(links.dirPath() == tmp.path() + QStringLiteral("/sub/dir"),
          "dirPath matches expanded custom dir");
    CHECK(links.includeCommand(QStringLiteral("0001.pdf"))
              == QStringLiteral("\\includegraphics{%1/sub/dir/0001.pdf}").arg(tmp.path()),
          "include command uses custom dir");

    // Empty/blank configuration falls back to the default.
    LinkManager blank(QStringLiteral("   "));
    CHECK(blank.configuredDir() == QStringLiteral("~/PicTikZ"),
          "blank configured dir falls back to default");
    qDebug() << "PASS: Test 2 - custom dir normalization and fallback";
}

static void testNextFreeLinkName()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    LinkManager links(tmp.path());

    // Empty directory -> 0001.pdf.
    CHECK(links.nextFreeLinkName() == QStringLiteral("0001.pdf"),
          "empty dir yields 0001.pdf");

    // 0001/0003/0004 exist -> the gap 0002 is filled first.
    CHECK(writeFile(tmp.path() + "/0001.pdf", QByteArray("1")), "0001.pdf created");
    CHECK(writeFile(tmp.path() + "/0003.pdf", QByteArray("3")), "0003.pdf created");
    CHECK(writeFile(tmp.path() + "/0004.pdf", QByteArray("4")), "0004.pdf created");
    CHECK(links.nextFreeLinkName() == QStringLiteral("0002.pdf"),
          "gap 0002.pdf filled before 0005.pdf");

    // Non-sequence files never occupy a number.
    CHECK(writeFile(tmp.path() + "/readme.txt", QByteArray("x")), "readme.txt created");
    CHECK(writeFile(tmp.path() + "/notes.pdf", QByteArray("x")), "notes.pdf created");
    CHECK(writeFile(tmp.path() + "/abc.png", QByteArray("x")), "abc.png created");
    CHECK(links.nextFreeLinkName() == QStringLiteral("0002.pdf"),
          "non-numeric names are ignored");

    // A dangling symlink still occupies its number.
    CHECK(QFile::link(tmp.path() + "/missing-target.pdf", tmp.path() + "/0002.pdf"),
          "dangling symlink 0002.pdf created");
    CHECK(!QFile::exists(tmp.path() + "/0002.pdf"), "0002.pdf target is missing");
    CHECK(links.linkEntryExists(QStringLiteral("0002.pdf")),
          "dangling symlink counts as an existing entry");
    CHECK(!links.linkTargetExists(QStringLiteral("0002.pdf")),
          "dangling symlink target does not exist");
    CHECK(links.nextFreeLinkName() == QStringLiteral("0005.pdf"),
          "dangling symlink keeps its number occupied");
    qDebug() << "PASS: Test 3 - sequence allocation with gap filling";
}

static void testCreateReplaceRemove()
{
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    LinkManager links(tmp.path());

    const QString src = tmp.path() + "/source.pdf";
    CHECK(writeFile(src, QByteArray("pdf-v1")), "source pdf created");

    // createLink makes the link dir automatically.
    CHECK(links.createLink(src, QStringLiteral("0001.pdf")), "createLink succeeds");
    CHECK(links.ensureDirExists(), "link dir exists after createLink");
    const QString linkPath = links.linkFilePath(QStringLiteral("0001.pdf"));
    CHECK(QFileInfo(linkPath).isSymLink(), "link file is a symlink");
    CHECK(links.linkEntryExists(QStringLiteral("0001.pdf")), "entry exists after create");
    CHECK(links.linkTargetExists(QStringLiteral("0001.pdf")), "target exists after create");

    if (QFileInfo(linkPath).isSymLink()) {
        CHECK(QFileInfo(linkPath).symLinkTarget() == QFileInfo(src).absoluteFilePath(),
              "symlink points at the source pdf");

        // The whole point of symlinks: overwriting the source is reflected
        // through the link without recreating anything.
        CHECK(writeFile(src, QByteArray("pdf-v2")), "source pdf updated");
        QFile lf(linkPath);
        CHECK(lf.open(QIODevice::ReadOnly), "link file readable");
        CHECK(lf.readAll() == QByteArray("pdf-v2"),
              "updated source content visible through the symlink");
        lf.close();
    }

    // createLink over an existing regular file replaces it.
    CHECK(writeFile(linkPath, QByteArray("stale-copy")), "regular file written over link");
    CHECK(links.createLink(src, QStringLiteral("0001.pdf")), "createLink replaces existing entry");
    CHECK(QFileInfo(linkPath).isSymLink(), "existing regular file replaced by symlink");

    // createLink with a missing source fails.
    CHECK(!links.createLink(tmp.path() + "/missing.pdf", QStringLiteral("0002.pdf")),
          "createLink with missing source fails");
    CHECK(!links.linkEntryExists(QStringLiteral("0002.pdf")),
          "no entry created for missing source");

    // removeLink frees the name again.
    CHECK(links.removeLink(QStringLiteral("0001.pdf")), "removeLink succeeds");
    CHECK(!links.linkEntryExists(QStringLiteral("0001.pdf")), "entry gone after removal");
    CHECK(links.nextFreeLinkName() == QStringLiteral("0001.pdf"),
          "freed number reused after removal");
    // Removing a non-existent name is a no-op success.
    CHECK(links.removeLink(QStringLiteral("9999.pdf")), "removeLink of missing name succeeds");
    qDebug() << "PASS: Test 4 - create/replace/remove links";
}

static void testCreateLinkFallbackCopy()
{
    // The copy fallback is exercised through the public API: force it by
    // checking that createLink still succeeds when symlinks are unavailable
    // is not possible on this platform, so instead verify that a created
    // link always exposes readable content equal to the source (symlink or
    // copy alike).
    QTemporaryDir tmp;
    CHECK(tmp.isValid(), "temp dir valid");
    LinkManager links(tmp.path());

    const QString src = tmp.path() + "/src.pdf";
    CHECK(writeFile(src, QByteArray(4096, 'z')), "source pdf created");
    CHECK(links.createLink(src, QStringLiteral("0001.pdf")), "createLink succeeds");

    const QString linkPath = links.linkFilePath(QStringLiteral("0001.pdf"));
    QFile lf(linkPath);
    CHECK(lf.open(QIODevice::ReadOnly), "link content readable");
    CHECK(lf.readAll() == QByteArray(4096, 'z'), "link content matches source");
    lf.close();
    qDebug() << "PASS: Test 5 - link content always matches source";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("hitikz-test-link-manager");
    QCoreApplication::setOrganizationName("HiTikZ");

    testDefaultDir();
    testCustomDirNormalization();
    testNextFreeLinkName();
    testCreateReplaceRemove();
    testCreateLinkFallbackCopy();

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) FAILED!";
        return 1;
    }
    qDebug() << "\nAll tests passed!";
    return 0;
}
