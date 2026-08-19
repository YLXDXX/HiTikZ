#include "mainwindow.h"
#include "search_panel.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "code_editor.h"
#include "tikz_document_state.h"
#include "link_manager.h"
#ifdef HAS_KGLOBALACCEL
#include "kde_global_shortcut.h"
#endif
#include "pdf_preview_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>
#include <QSplitter>
#include <QApplication>
#include <QStatusBar>
#include <QHeaderView>
#include <QIcon>
#include <QFileDialog>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QFileInfo>
#include <QClipboard>
#include <QImage>
#include <QShortcut>
#include <QMenu>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QEventLoop>
#include <QTimer>
#include <QMimeData>
#include <QDataStream>
#include <QDropEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QTabBar>
#include <QCheckBox>
#include <memory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QScreen>
#include <QtConcurrent>
#include <QThreadPool>
#include "mainwindow_internal.h"

// ── 复制链接 ─────────────────────────────────────────────────────────────
//
// Symlinks the current snippet's preview PDF into the shared picture
// directory (default ~/PicTikZ, configurable in the settings dialog) under a
// sequence number name (0001.pdf, 0002.pdf, ...), then copies the matching
// \includegraphics command to the clipboard. Because the link points at the
// snippet's own preview.pdf, recompiling the snippet automatically updates
// the picture in every LaTeX document that references it.
//
// The recorded file name is persisted in the snippet's meta.json
// ("linkedPdf") so repeated clicks keep the same link instead of creating
// new ones.
void MainWindow::copyLinkToPicTikZ()
{
    // The link file name must be persisted in the snippet's meta.json, which
    // does not exist for unsaved scratch tabs.
    if (currentSnippetId.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("请先保存片段，再使用复制链接"), kStatusBarShortMs);
        return;
    }

    Snippet s = snippetMgr->loadSnippet(currentSnippetId);
    if (s.id.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("无法加载当前片段"), kStatusBarShortMs);
        return;
    }

    // Prefer the snippet's own preview.pdf: it is rewritten on every
    // successful compile, so a symlink to it stays up to date.
    QString pdfPath = snippetDataPath(currentSnippetId) + "/preview.pdf";
    if (!QFile::exists(pdfPath))
        pdfPath = compiler->pdfPath();
    if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
        statusBar()->showMessage(QStringLiteral("请先编译生成PDF预览"), kStatusBarShortMs);
        return;
    }

    LinkManager links(LinkManager::settingDir());
    if (!links.ensureDirExists()) {
        statusBar()->showMessage(QStringLiteral("无法创建链接目录: %1").arg(links.dirPath()),
                                 kStatusBarLongMs);
        return;
    }

    // Requirement: reuse the recorded name when present; only allocate a new
    // sequence number (filling gaps) when nothing is recorded yet.
    QString linkName = s.linkedPdf;
    if (linkName.isEmpty())
        linkName = links.nextFreeLinkName();
    if (linkName.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("无法分配超链接文件名"), kStatusBarLongMs);
        return;
    }

    // Existing link whose target is intact: nothing to create. A dangling
    // link (or a missing recorded file) is recreated pointing at the current
    // preview PDF.
    if (!links.linkTargetExists(linkName)) {
        if (!links.createLink(pdfPath, linkName)) {
            statusBar()->showMessage(QStringLiteral("无法创建超链接文件"), kStatusBarLongMs);
            return;
        }
    }

    if (s.linkedPdf != linkName) {
        s.linkedPdf = linkName;
        snippetMgr->saveSnippet(s);
    }

    const QString command = links.includeCommand(linkName);
    QApplication::clipboard()->setText(command);
    statusBar()->showMessage(QStringLiteral("链接已复制: %1").arg(command), kStatusBarLongMs);
}
