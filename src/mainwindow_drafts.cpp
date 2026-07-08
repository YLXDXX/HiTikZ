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

void MainWindow::clearDraft()
{
    QString draftDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/drafts/";
    if (currentSnippetId.isEmpty()) {
        for (int i = 0; i < tabWidget->count(); ++i) {
            if (tabWidget->currentIndex() == i) {
                QFile::remove(draftDir + QStringLiteral("scratch_%1.json").arg(i));
                break;
            }
        }
    } else {
        QFile::remove(draftDir + currentSnippetId + ".json");
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
    QDir d(draftDir);
    if (!d.exists()) return;

    QStringList draftFiles = d.entryList(QStringList() << "*.json", QDir::Files);
    if (draftFiles.isEmpty()) return;

    struct DraftInfo {
        QString filePath;
        QString snippetId;
        QString name;
        QString code;
        QString description;
        QString tags;
        QString packages;
        QString tikzLibraries;
        QString templateId;
    };

    QList<DraftInfo> drafts;
    for (const QString &fileName : draftFiles) {
        QString filePath = draftDir + fileName;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) continue;

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (!doc.isObject()) continue;

        QJsonObject obj = doc.object();
        DraftInfo info;
        info.filePath = filePath;
        info.snippetId = obj.value("snippetId").toString();
        info.name = obj.value("name").toString();
        info.code = obj.value("code").toString();
        info.description = obj.value("description").toString();
        info.tags = obj.value("tags").toString();
        info.packages = obj.value("packages").toString();
        info.tikzLibraries = obj.value("tikzLibraries").toString();
        info.templateId = obj.value("templateId").toString();

        if (info.name.isEmpty())
            info.name = info.snippetId.isEmpty() ? QStringLiteral("未命名草稿") : info.snippetId.left(8);
        if (info.code.trimmed().isEmpty()) continue;

        drafts.append(info);
    }

    if (drafts.isEmpty()) return;

    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("恢复草稿"));
    dlg.setMinimumSize(450, 300);
    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QLabel *infoLabel = new QLabel(
        QStringLiteral("检测到 %1 个未保存的草稿。\n请选择要恢复的草稿：").arg(drafts.size()));
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    QScrollArea *scroll = new QScrollArea;
    QWidget *scrollWidget = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setContentsMargins(0, 0, 0, 0);

    QList<QCheckBox *> checkboxes;
    for (const DraftInfo &draft : drafts) {
        QString label = draft.name;
        if (!draft.description.isEmpty())
            label += QStringLiteral(" — %1").arg(draft.description.left(60));
        QCheckBox *cb = new QCheckBox(label);
        cb->setChecked(true);
        cb->setProperty("draftIndex", checkboxes.size());
        scrollLayout->addWidget(cb);
        checkboxes.append(cb);
    }
    scrollLayout->addStretch();

    QPushButton *selectAllBtn = new QPushButton(QStringLiteral("全选"));
    QPushButton *deselectAllBtn = new QPushButton(QStringLiteral("取消全选"));
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(selectAllBtn);
    btnRow->addWidget(deselectAllBtn);
    btnRow->addStretch();
    scrollLayout->addLayout(btnRow);

    connect(selectAllBtn, &QPushButton::clicked, &dlg, [&checkboxes]() {
        for (QCheckBox *cb : checkboxes) cb->setChecked(true);
    });
    connect(deselectAllBtn, &QPushButton::clicked, &dlg, [&checkboxes]() {
        for (QCheckBox *cb : checkboxes) cb->setChecked(false);
    });

    scroll->setWidget(scrollWidget);
    scroll->setWidgetResizable(true);
    layout->addWidget(scroll, 1);

    QDialogButtonBox *btnBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Discard);
    btnBox->button(QDialogButtonBox::Ok)->setText(QStringLiteral("恢复所选"));
    btnBox->button(QDialogButtonBox::Discard)->setText(QStringLiteral("全部丢弃"));
    layout->addWidget(btnBox);

    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, &dlg, [&dlg]() {
        dlg.done(QDialog::Rejected + 1);
    });

    int result = dlg.exec();
    QList<int> recoveredIndices;
    if (result == QDialog::Accepted) {
        for (int i = 0; i < checkboxes.size(); ++i) {
            if (checkboxes[i]->isChecked())
                recoveredIndices.append(i);
        }
    }

    for (int idx : recoveredIndices) {
        const DraftInfo &draft = drafts[idx];
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

            if (draft.snippetId.isEmpty()) {
                snippetMgr->saveSnippet(s);
                currentSnippetId = s.id;
            } else {
                snippetMgr->saveSnippet(s);
                currentSnippetId = s.id;
            }
            createNewTab(s.id, draft.code, draft.name);
        } else {
            loadSnippetIntoEditor(draft.snippetId);
        }
        QFile::remove(draft.filePath);
    }

    if (result == QDialog::Rejected + 1) {
        for (const DraftInfo &draft : drafts)
            QFile::remove(draft.filePath);
    }

    if (!recoveredIndices.isEmpty()) {
        refreshCategoryTree();
        refreshSearch();
    }
}
