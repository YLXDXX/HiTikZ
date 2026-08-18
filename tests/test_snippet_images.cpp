// Tests for the snippet image feature:
//  - letter-sequence naming (a.png, b.png, ..., z.png, aa.png, ...)
//  - supported image extension detection
//  - add / replace / remove image files via SnippetManager
//  - meta.json "images" field roundtrip + tolerance of missing field
//  - export/import archives carry images along
//  - copying image files between snippets (duplicate)
//  - LatexCompiler copies images into the compile temp dir
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "snippet_properties_dialog.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QClipboard>
#include <QImage>
#include <QPushButton>
#include <QListWidget>
#include <QKeyEvent>
#include <QDebug>

static int g_failed = 0;

#define CHECK(expr, msg) \
    do { \
        if (!(expr)) { \
            qDebug() << "FAIL:" << msg; \
            g_failed++; \
        } \
    } while (0)

static bool writeDummyFile(const QString &path, const QByteArray &content)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        return false;
    f.write(content);
    f.close();
    return true;
}

static QString makeDummyImage(const QString &dir, const QString &name, const QByteArray &content)
{
    const QString path = dir + "/" + name;
    return writeDummyFile(path, content) ? path : QString();
}

static void testImageStemGenerator()
{
    CHECK(SnippetManager::imageStemForIndex(0) == "a", "stem 0 -> a");
    CHECK(SnippetManager::imageStemForIndex(1) == "b", "stem 1 -> b");
    CHECK(SnippetManager::imageStemForIndex(25) == "z", "stem 25 -> z");
    CHECK(SnippetManager::imageStemForIndex(26) == "aa", "stem 26 -> aa");
    CHECK(SnippetManager::imageStemForIndex(27) == "ab", "stem 27 -> ab");
    CHECK(SnippetManager::imageStemForIndex(51) == "az", "stem 51 -> az");
    CHECK(SnippetManager::imageStemForIndex(52) == "ba", "stem 52 -> ba");
    CHECK(SnippetManager::imageStemForIndex(701) == "zz", "stem 701 -> zz");
    CHECK(SnippetManager::imageStemForIndex(-7) == "a", "negative stem -> a");
    qDebug() << "PASS: Test 1 - image stem generator";
}

static void testSupportedExtensions()
{
    CHECK(SnippetManager::isSupportedImageFile("a.png"), "a.png supported");
    CHECK(SnippetManager::isSupportedImageFile("B.PNG"), "uppercase .PNG supported");
    CHECK(SnippetManager::isSupportedImageFile("c.jpg"), "c.jpg supported");
    CHECK(SnippetManager::isSupportedImageFile("d.jpeg"), "d.jpeg supported");
    CHECK(SnippetManager::isSupportedImageFile("e.tif"), "e.tif supported");
    CHECK(SnippetManager::isSupportedImageFile("f.tiff"), "f.tiff supported");
    CHECK(SnippetManager::isSupportedImageFile("g.bmp"), "g.bmp supported");
    CHECK(SnippetManager::isSupportedImageFile("h.gif"), "h.gif supported");
    CHECK(SnippetManager::isSupportedImageFile("i.webp"), "i.webp supported");
    CHECK(SnippetManager::isSupportedImageFile("j.pdf"), "j.pdf supported");
    CHECK(!SnippetManager::isSupportedImageFile("k.txt"), "k.txt not supported");
    CHECK(!SnippetManager::isSupportedImageFile("nosuffix"), "no suffix not supported");
    CHECK(!SnippetManager::isSupportedImageFile("l.svg"), "l.svg not supported");
    qDebug() << "PASS: Test 2 - supported image extensions";
}

static void testAddListRemove(SnippetManager &mgr)
{
    QTemporaryDir srcDir;
    CHECK(srcDir.isValid(), "temporary source dir valid");
    const QString png1 = makeDummyImage(srcDir.path(), "photo1.png", QByteArray(64, '1'));
    const QString pdf1 = makeDummyImage(srcDir.path(), "fig1.pdf", QByteArray(96, '2'));
    const QString jpg1 = makeDummyImage(srcDir.path(), "scan1.jpg", QByteArray(32, '3'));
    CHECK(!png1.isEmpty() && !pdf1.isEmpty() && !jpg1.isEmpty(), "dummy source files created");

    const QString id = mgr.createSnippet("ImageTest", "test/images");
    CHECK(!id.isEmpty(), "create snippet for image test");

    // Add three images -> a.png, b.pdf, c.jpg
    const QString n1 = mgr.addImageToSnippet(id, png1);
    const QString n2 = mgr.addImageToSnippet(id, pdf1);
    const QString n3 = mgr.addImageToSnippet(id, jpg1);
    CHECK(n1 == "a.png", "first image named a.png");
    CHECK(n2 == "b.pdf", "second image named b.pdf");
    CHECK(n3 == "c.jpg", "third image named c.jpg");

    const QString imgDir = mgr.getSnippetImageDir(id);
    CHECK(QFile::exists(imgDir + "/a.png"), "a.png exists in snippet dir");
    CHECK(QFile::exists(imgDir + "/b.pdf"), "b.pdf exists in snippet dir");
    CHECK(QFile::exists(imgDir + "/c.jpg"), "c.jpg exists in snippet dir");

    Snippet s = mgr.loadSnippet(id);
    CHECK(s.images == QStringList({"a.png", "b.pdf", "c.jpg"}), "meta.json images roundtrip");
    CHECK(mgr.getSnippetImagePaths(id).size() == 3, "three image paths returned");

    // Removing b.pdf frees the stem "b"; the next add reuses it.
    CHECK(mgr.removeSnippetImage(id, "b.pdf"), "remove b.pdf succeeds");
    CHECK(!QFile::exists(imgDir + "/b.pdf"), "b.pdf file gone after removal");
    s = mgr.loadSnippet(id);
    CHECK(s.images == QStringList({"a.png", "c.jpg"}), "meta.json updated after removal");

    const QString png2 = makeDummyImage(srcDir.path(), "photo2.png", QByteArray(16, '4'));
    const QString n4 = mgr.addImageToSnippet(id, png2);
    CHECK(n4 == "b.png", "freed stem b reused as b.png");

    // Stems listed in meta.json are skipped even if their files are missing,
    // and files on disk (c.jpg) are never overwritten even when unlisted.
    s = mgr.loadSnippet(id);
    s.images = QStringList{"a.png", "b.png", "x.pdf", "y.pdf"};
    mgr.saveSnippet(s);
    const QString n5 = mgr.addImageToSnippet(id, png2);
    CHECK(n5 == "d.png", "next free stem d.png chosen (meta + on-disk stems skipped)");
    // Listed now: a.png, b.png, x.pdf, y.pdf, d.png — x/y files don't exist and
    // the on-disk c.jpg is unlisted, so only 3 paths are returned.
    CHECK(mgr.getSnippetImagePaths(id).size() == 3,
          "only listed-and-existing image files are returned as paths (x.pdf/y.pdf filtered, c.jpg unlisted)");

    mgr.deleteSnippet(id);
    qDebug() << "PASS: Test 3 - add/list/remove images";
}

static void testReplace(SnippetManager &mgr)
{
    QTemporaryDir srcDir;
    CHECK(srcDir.isValid(), "temporary source dir valid");
    const QString pngSmall = makeDummyImage(srcDir.path(), "small.png", QByteArray(16, 'x'));
    const QString pngBig = makeDummyImage(srcDir.path(), "big.png", QByteArray(1024, 'y'));
    const QString jpgFile = makeDummyImage(srcDir.path(), "replacement.jpg", QByteArray(512, 'z'));
    CHECK(!pngSmall.isEmpty() && !pngBig.isEmpty() && !jpgFile.isEmpty(), "dummy files created");

    const QString id = mgr.createSnippet("ReplaceTest", "test/images");
    const QString name = mgr.addImageToSnippet(id, pngSmall);
    CHECK(name == "a.png", "a.png added for replace test");
    const QString imgDir = mgr.getSnippetImageDir(id);

    // Replace with same extension: name kept, content swapped.
    const QString n1 = mgr.replaceSnippetImage(id, "a.png", pngBig);
    CHECK(n1 == "a.png", "replace with same extension keeps name");
    CHECK(QFile(imgDir + "/a.png").size() == 1024, "a.png content replaced");

    // Replace with a different extension: stem kept, extension updated.
    const QString n2 = mgr.replaceSnippetImage(id, "a.png", jpgFile);
    CHECK(n2 == "a.jpg", "replace with jpg renames to a.jpg");
    CHECK(!QFile::exists(imgDir + "/a.png"), "old a.png removed");
    CHECK(QFile::exists(imgDir + "/a.jpg"), "new a.jpg exists");
    Snippet s = mgr.loadSnippet(id);
    CHECK(s.images == QStringList({"a.jpg"}), "meta.json updated after replace");

    // Replacing an unknown image appends instead of crashing.
    const QString n3 = mgr.replaceSnippetImage(id, "ghost.png", pngSmall);
    CHECK(n3 == "ghost.png", "replacing unlisted image keeps its stem");
    CHECK(QFile::exists(imgDir + "/ghost.png"), "ghost.png file created");

    // Replacing with a missing source fails and keeps the old file.
    const QString n4 = mgr.replaceSnippetImage(id, "ghost.png", srcDir.path() + "/missing.png");
    CHECK(n4.isEmpty(), "replace with missing source fails");
    CHECK(QFile::exists(imgDir + "/ghost.png"), "old file kept after failed replace");

    mgr.deleteSnippet(id);
    qDebug() << "PASS: Test 4 - replace images";
}

static void testJsonTolerance(SnippetManager &mgr)
{
    const QString id = mgr.createSnippet("JsonTolTest", "test/images");
    QTemporaryDir srcDir;
    const QString png = makeDummyImage(srcDir.path(), "one.png", QByteArray(8, 'q'));
    mgr.addImageToSnippet(id, png);

    // Simulate a meta.json written by an older version (no "images" field).
    const QString metaPath = mgr.getBasePath() + id + "/meta.json";
    {
        QFile f(metaPath);
        CHECK(f.open(QIODevice::ReadOnly), "meta.json readable");
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        QJsonObject obj = doc.object();
        obj.remove("images");
        QSaveFile out(metaPath);
        CHECK(out.open(QIODevice::WriteOnly), "meta.json writable");
        out.write(QJsonDocument(obj).toJson());
        out.commit();
    }
    Snippet s = mgr.loadSnippet(id);
    CHECK(s.images.isEmpty(), "missing images field tolerated (empty list)");

    // Saving again writes the field back (with current in-memory value).
    mgr.saveSnippet(s);
    s = mgr.loadSnippet(id);
    CHECK(s.images.isEmpty(), "images field written back on save");

    mgr.deleteSnippet(id);
    qDebug() << "PASS: Test 5 - meta.json tolerance for missing images field";
}

static void testExportImportImages(SnippetManager &mgr)
{
    const QString id = mgr.createSnippet("ExportImgTest", "test/images");
    QTemporaryDir srcDir;
    const QString png = makeDummyImage(srcDir.path(), "exp.png", QByteArray(64, 'e'));
    const QString name = mgr.addImageToSnippet(id, png);
    CHECK(name == "a.png", "image added for export test");

    const QString zipPath = QDir::tempPath() + "/test_images_export.tar.gz";
    if (QFile::exists(zipPath))
        QFile::remove(zipPath);

    bool exportOk = mgr.exportSnippetZip(id, zipPath);
    CHECK(exportOk, "export with images succeeds");
    CHECK(QFile::exists(zipPath), "archive exists");

    const QStringList importedIds = mgr.importSnippetsZip(zipPath);
    CHECK(importedIds.size() == 1, "one snippet imported");
    if (!importedIds.isEmpty()) {
        const QString newId = importedIds.first();
        Snippet imported = mgr.loadSnippet(newId);
        CHECK(imported.images == QStringList({"a.png"}), "images field survives export/import");
        CHECK(QFile::exists(mgr.getBasePath() + newId + "/a.png"),
              "image file extracted next to imported snippet");
        mgr.deleteSnippet(newId);
    }

    QFile::remove(zipPath);
    mgr.deleteSnippet(id);
    qDebug() << "PASS: Test 6 - export/import carries images";
}

static void testCopyImageFiles(SnippetManager &mgr)
{
    const QString srcId = mgr.createSnippet("CopySrc", "test/images");
    const QString dstId = mgr.createSnippet("CopyDst", "test/images");
    QTemporaryDir srcDir;
    const QString png = makeDummyImage(srcDir.path(), "src.png", QByteArray(48, 'c'));
    const QString name = mgr.addImageToSnippet(srcId, png);
    CHECK(name == "a.png", "source image added");

    // Simulate the duplicate-snippet flow in MainWindow.
    Snippet dup = mgr.loadSnippet(dstId);
    dup.images = mgr.loadSnippet(srcId).images;
    const bool copied = mgr.copySnippetImageFiles(srcId, dstId);
    mgr.saveSnippet(dup);
    CHECK(copied, "copySnippetImageFiles succeeds");
    CHECK(QFile::exists(mgr.getBasePath() + dstId + "/a.png"),
          "image file copied into destination snippet dir");
    CHECK(mgr.getSnippetImagePaths(dstId).size() == 1,
          "destination snippet reports the copied image");

    mgr.deleteSnippet(srcId);
    mgr.deleteSnippet(dstId);
    qDebug() << "PASS: Test 7 - copy image files between snippets";
}

static void testCompileCopiesImages(LatexCompiler &compiler)
{
    QTemporaryDir srcDir;
    const QString img = makeDummyImage(srcDir.path(), "copyme.png", QByteArray(32, 't'));
    CHECK(!img.isEmpty(), "dummy image created");

    QString pdfPath, log;
    // Trivial snippet; compile success depends on xelatex availability, but the
    // image copy happens synchronously before the process is started.
    compiler.compileBlocking("\\begin{tikzpicture}\\end{tikzpicture}", QString(),
                             "imgtest", QString(), QString(), 20000, pdfPath, log,
                             QString(), QStringList{img});

    const QString copied = compiler.tempDirPath() + "/imgtest/copyme.png";
    CHECK(QFile::exists(copied), "image copied into compile temp dir");
    CHECK(QFile(copied).size() == 32, "copied image content matches");

    QDir(compiler.tempDirPath() + "/imgtest").removeRecursively();
    qDebug() << "PASS: Test 8 - compiler copies snippet images";
}

static void testPropertiesDialogImages(SnippetManager &mgr)
{
    const QString id = mgr.createSnippet("DialogTest", "test/images");
    QTemporaryDir srcDir;
    const QString png = makeDummyImage(srcDir.path(), "dlg.png", QByteArray(24, 'd'));
    CHECK(!png.isEmpty(), "dummy image created");
    CHECK(mgr.addImageToSnippet(id, png) == "a.png", "a.png added");
    CHECK(mgr.addImageToSnippet(id, png) == "b.png", "b.png added");

    SnippetPropertiesDialog dlg(id, &mgr);
    const QList<QListWidget*> lists = dlg.findChildren<QListWidget*>();
    CHECK(lists.size() == 1, "properties dialog has one image list");
    QListWidget *list = lists.isEmpty() ? nullptr : lists.first();
    if (list) {
        CHECK(list->count() == 2, "image list shows both images");
        CHECK(list->item(0)->text() == "a.png", "first item a.png");
        CHECK(list->item(1)->text() == "b.png", "second item b.png");
    }

    auto findBtn = [&](const QString &text) -> QPushButton* {
        const auto btns = dlg.findChildren<QPushButton*>();
        for (QPushButton *b : btns)
            if (b->text() == text)
                return b;
        return nullptr;
    };
    QPushButton *viewBtn = findBtn(QStringLiteral("查看"));
    QPushButton *removeBtn = findBtn(QStringLiteral("删除"));
    QPushButton *replaceBtn = findBtn(QStringLiteral("替换..."));
    QPushButton *copyNameBtn = findBtn(QStringLiteral("复制文件名"));
    CHECK(findBtn(QStringLiteral("导入图片...")) != nullptr, "import button exists");
    CHECK(findBtn(QStringLiteral("粘贴图片 (Ctrl+V)")) != nullptr, "paste button exists");

    // On opening the dialog the selection-dependent buttons are disabled even
    // though images exist — they activate only once an image is selected.
    CHECK(viewBtn != nullptr && !viewBtn->isEnabled(),
          "view button disabled before any image is selected");
    CHECK(removeBtn != nullptr && !removeBtn->isEnabled(),
          "remove button disabled before any image is selected");
    CHECK(replaceBtn != nullptr && !replaceBtn->isEnabled(),
          "replace button disabled before any image is selected");
    CHECK(copyNameBtn != nullptr && !copyNameBtn->isEnabled(),
          "copy-name button disabled before any image is selected");

    if (list) {
        list->setCurrentRow(0);
        QApplication::processEvents();
    }
    CHECK(viewBtn->isEnabled(), "view button enabled after selecting an image");
    CHECK(removeBtn->isEnabled(), "remove button enabled after selecting an image");
    CHECK(replaceBtn->isEnabled(), "replace button enabled after selecting an image");
    CHECK(copyNameBtn->isEnabled(), "copy-name button enabled after selecting an image");

    // Switching the selection away disables the buttons again.
    if (list) {
        list->setCurrentRow(-1);
        QApplication::processEvents();
    }
    CHECK(!viewBtn->isEnabled(), "view button disabled when selection cleared");

    // Paste an image from the clipboard via the "onPasteImage" slot.
    QImage clipImage(4, 4, QImage::Format_RGB32);
    clipImage.fill(Qt::red);
    QApplication::clipboard()->setImage(clipImage);
    QMetaObject::invokeMethod(&dlg, "onPasteImage", Qt::DirectConnection);
    if (list) {
        CHECK(list->count() == 3, "pasted clipboard image appears in the list");
        if (list->count() == 3)
            CHECK(list->item(2)->text() == "c.png", "pasted image named c.png");
    }
    // The list is rebuilt after pasting, so the selection (and the
    // selection-dependent buttons) reset to disabled.
    CHECK(!viewBtn->isEnabled(), "view button disabled after list refresh");

    // Ctrl+V is routed to image paste when no text widget has focus.
    clipImage.fill(Qt::blue);
    QApplication::clipboard()->setImage(clipImage);
    QKeyEvent pasteEvent(QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier);
    QApplication::sendEvent(&dlg, &pasteEvent);
    if (list) {
        CHECK(list->count() == 4, "Ctrl+V pastes another clipboard image");
        if (list->count() == 4)
            CHECK(list->item(3)->text() == "d.png", "second pasted image named d.png");
    }

    // Selecting one of the new images re-enables the buttons.
    if (list) {
        list->setCurrentRow(2);
        QApplication::processEvents();
    }
    CHECK(viewBtn->isEnabled(), "view button enabled after selecting pasted image");

    Snippet s = mgr.loadSnippet(id);
    CHECK(s.images.size() == 4, "meta.json lists all four images");

    mgr.deleteSnippet(id);
    qDebug() << "PASS: Test 9 - properties dialog image management";
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("hitikz-test-images");
    QCoreApplication::setOrganizationName("HiTikZ");
    QStandardPaths::setTestModeEnabled(true);

    testImageStemGenerator();
    testSupportedExtensions();

    {
        SnippetManager mgr;
        testAddListRemove(mgr);
        testReplace(mgr);
        testJsonTolerance(mgr);
        testExportImportImages(mgr);
        testCopyImageFiles(mgr);
        testPropertiesDialogImages(mgr);
    }

    {
        LatexCompiler compiler;
        testCompileCopiesImages(compiler);
    }

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) FAILED!";
        return 1;
    }
    qDebug() << "\nAll tests passed!";
    return 0;
}
