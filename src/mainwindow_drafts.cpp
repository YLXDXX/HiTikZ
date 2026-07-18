#include "mainwindow.h"
#include "search_panel.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "code_editor.h"
#include "tikz_document_state.h"
#include "settings_dialog.h"
#ifdef HAS_KGLOBALACCEL
#include "kde_global_shortcut.h"
#endif
#include "pdf_preview_widget.h"
#include "draft_recovery_dialog.h"
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

void MainWindow::startAutoSave()
{
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setInterval(kAutoSaveIntervalMs);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::performAutoSave);
    autoSaveTimer->start();
}

void MainWindow::performAutoSave()
{
    // While draft recovery is still pending (hidden autostart: the prompt is
    // deferred until the window is first shown), the files on disk are crash
    // leftovers the user has not seen yet — do not touch them.
    if (m_pendingDraftRecovery)
        return;

    QString draftDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/drafts/";
    {
        QDir dir(draftDir);
        QStringList oldScratches = dir.entryList(QStringList() << "scratch_*.json", QDir::Files);
        for (const QString &f : oldScratches)
            QFile::remove(draftDir + f);
    }
    QDir().mkpath(draftDir);

    for (int i = 0; i < tabWidget->count(); ++i) {
        CodeEditor *ed = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (!ed || ed->toPlainText().trimmed().isEmpty())
            continue;

        QString sid = tabWidget->tabBar()->tabData(i).toString();

        QString draftPath = draftDir + (sid.isEmpty()
            ? QStringLiteral("scratch_%1").arg(i) : sid) + ".json";

        QJsonObject obj;
        obj["snippetId"] = sid;
        obj["code"] = ed->toPlainText();
        obj["name"] = tabWidget->tabText(i);

        if (i == tabWidget->currentIndex()) {
            // The active tab's metadata widgets hold the freshest (possibly
            // unsaved) values, so persist those into the draft instead of the
            // stale on-disk metadata.
            obj["description"] = descEdit->toPlainText();
            QStringList tags;
            for (const QString &tag : tagsEdit->text().split(',')) {
                QString t = tag.trimmed();
                if (!t.isEmpty()) tags.append(t);
            }
            obj["tags"] = tags.join(QStringLiteral(", "));
            obj["packages"] = packagesEdit->text();
            obj["tikzLibraries"] = tikzLibrariesEdit->text();
            obj["templateId"] = templateCombo->currentData().toString();
        } else if (!sid.isEmpty()) {
            Snippet s = snippetMgr->loadSnippet(sid);
            obj["description"] = s.description;
            obj["tags"] = s.tags.join(QStringLiteral(", "));
            obj["packages"] = s.packages;
            obj["tikzLibraries"] = s.tikzLibraries;
            obj["templateId"] = s.templateId;
        } else {
            obj["description"] = QString();
            obj["tags"] = QString();
            obj["packages"] = QString();
            obj["tikzLibraries"] = QString();
            obj["templateId"] = QString();
        }

        QSaveFile file(draftPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(obj).toJson());
            file.commit();
        }
    }
}

void MainWindow::clearDraft(int tabIndex)
{
    QString draftDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/drafts/";
    int idx = (tabIndex >= 0) ? tabIndex : tabWidget->currentIndex();
    if (idx < 0 || idx >= tabWidget->count()) return;

    QString sid = tabWidget->tabBar()->tabData(idx).toString();
    if (sid.isEmpty()) {
        QFile::remove(draftDir + QStringLiteral("scratch_%1.json").arg(idx));
    } else {
        QFile::remove(draftDir + sid + ".json");
    }
}

void MainWindow::clearAllDrafts()
{
    QString draftDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/drafts/";
    QDir d(draftDir);
    if (!d.exists()) return;

    QStringList drafts = d.entryList(QStringList() << "*.json", QDir::Files);
    for (const QString &draft : drafts)
        QFile::remove(draftDir + draft);
}

void MainWindow::recoverDrafts()
{
    QString draftDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/drafts/";
    const QList<DraftRecoveryDialog::Draft> drafts =
        DraftRecoveryDialog::loadDraftsFromDir(draftDir);
    if (drafts.isEmpty()) return;

    DraftRecoveryDialog dlg(drafts, this);
    const int result = dlg.exec();

    if (result != DraftRecoveryDialog::RecoverSelected
        && result != DraftRecoveryDialog::DiscardAll) {
        // "稍后处理" / Esc / 关闭：保留草稿文件，下次启动再询问。
        return;
    }

    const QList<int> recoveredIndices =
        (result == DraftRecoveryDialog::RecoverSelected) ? dlg.selectedIndices()
                                                         : QList<int>();

    for (int idx : recoveredIndices) {
        const DraftRecoveryDialog::Draft &draft = drafts[idx];
        if (draft.snippetId.isEmpty() || !snippetMgr->snippetExists(draft.snippetId)) {
            Snippet s;
            s.id = draft.snippetId.isEmpty()
                ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                : draft.snippetId;
            s.name = draft.name;
            s.description = draft.description;
            s.code = draft.code;
            QStringList tags;
            for (const QString &tag : draft.tags.split(',')) {
                QString trimmed = tag.trimmed();
                if (!trimmed.isEmpty()) tags.append(trimmed);
            }
            s.tags = tags;
            s.packages = draft.packages;
            s.tikzLibraries = draft.tikzLibraries;
            s.templateId = draft.templateId;

            snippetMgr->saveSnippet(s);
            currentSnippetId = s.id;
            createNewTab(s.id, draft.code, draft.name);
        } else {
            loadSnippetIntoEditor(draft.snippetId);
        }
    }

    // 恢复所选：已恢复的内容进入片段库，未勾选的是用户明确放弃的；
    // 全部丢弃：全部删除。两种情况都清空草稿文件，避免每次启动重复弹窗。
    for (const DraftRecoveryDialog::Draft &draft : drafts)
        QFile::remove(draft.filePath);

    if (!recoveredIndices.isEmpty()) {
        refreshCategoryTree();
        refreshSearch();
    }
}
