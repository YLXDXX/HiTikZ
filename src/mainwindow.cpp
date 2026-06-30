#include "mainwindow.h"
#include "search_panel.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "code_editor.h"
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

#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define STRINGIFY_IMPL(x) #x
#define RES_DIR STRINGIFY(RESOURCE_DIR)

static constexpr int kAutoSaveIntervalMs = 180000;
static constexpr int kStatusBarShortMs = 3000;
static constexpr int kStatusBarLongMs = 5000;
static constexpr int kPreviewDpi = 150;
static constexpr int kBatchCompileTimeoutMs = 30000;
static constexpr int kPngConvertTimeoutMs = 15000;
static constexpr int kZoomApplyDelayMs = 200;
static constexpr int kDefaultFontSize = 10;

static const QRegularExpression paramLineRe(QStringLiteral("^%\\s*@param:.*(\n|\r\n?)?"), QRegularExpression::MultilineOption);

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QString resourceDir = QStringLiteral(RES_DIR);
    SettingsDialog::ensureTemplatesCopied(resourceDir + "/templates");
    SnippetManager::copyPresetsFromResources(
        resourceDir + "/presets",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/presets/");
    snippetMgr = new SnippetManager(this);
    compiler = new LatexCompiler(this);
    SettingsDialog::applyToCompiler(compiler);

    setupUI();
    setupConnections();
    refreshTemplateCombo();
    refreshCategoryTree();
    refreshSearch();

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon::fromTheme("applications-graphics"));
    trayIcon->setToolTip(QStringLiteral("HiTikZ - TikZ 代码管理器"));

    trayMenu = new QMenu(this);
    QAction *showAct = trayMenu->addAction(QStringLiteral("显示窗口"));
    QAction *hideAct = trayMenu->addAction(QStringLiteral("隐藏到托盘"));
    trayMenu->addSeparator();
    QAction *quitAct = trayMenu->addAction(QStringLiteral("退出"));

    connect(showAct, &QAction::triggered, this, [this]() {
        show();
        raise();
        activateWindow();
        searchPanel->setFocus();
    });
    connect(hideAct, &QAction::triggered, this, &QMainWindow::hide);
    connect(quitAct, &QAction::triggered, this, [this]() {
        m_forceQuit = true;
        close();
    });
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
            if (isVisible() && !isMinimized()) {
                hide();
            } else {
                show();
                raise();
                activateWindow();
                searchPanel->setFocus();
            }
        }
    });

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    QTimer::singleShot(100, this, [this]() {
        checkSystemDependencies();
    });

    applyGlobalHotkey();
    startAutoSave();
    QTimer::singleShot(300, this, [this]() {
        recoverDrafts();
    });
}

MainWindow::~MainWindow()
{
}

CodeEditor *MainWindow::currentEditor() const
{
    if (!tabWidget || tabWidget->count() == 0)
        return nullptr;
    return qobject_cast<CodeEditor*>(tabWidget->currentWidget());
}

QString MainWindow::currentTabSnippetId() const
{
    int idx = tabWidget ? tabWidget->currentIndex() : -1;
    if (idx < 0) return QString();
    return tabWidget->tabBar()->tabData(idx).toString();
}

int MainWindow::findTabForSnippet(const QString &id) const
{
    if (id.isEmpty() || !tabWidget) return -1;
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabBar()->tabData(i).toString() == id)
            return i;
    }
    return -1;
}

void MainWindow::updateTabTitle(int index, const QString &title)
{
    if (index < 0 || index >= tabWidget->count()) return;
    tabWidget->setTabText(index, title);
}

void MainWindow::connectEditorSignals(CodeEditor *editor)
{
    connect(editor, &QPlainTextEdit::textChanged, this, [this]() {
        if (m_loadingDepth == 0)
            onCurrentSnippetChanged();
    });
}

void MainWindow::createNewTab(const QString &snippetId, const QString &code,
                              const QString &title)
{
    if (!tabWidget) return;

    CodeEditor *editor = new CodeEditor;
    editor->setTabStopDistance(4 * editor->fontMetrics().horizontalAdvance(' '));
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    connectEditorSignals(editor);

    tabWidget->blockSignals(true);
    int idx = tabWidget->addTab(editor, title);
    tabWidget->tabBar()->setTabData(idx, snippetId);
    tabWidget->blockSignals(false);
    tabWidget->setCurrentIndex(idx);

    if (!code.isNull()) {
        m_loadingDepth++;
        const QSignalBlocker blocker(editor);
        editor->setPlainText(code);
        m_loadingDepth--;
    }

    applyAppearanceSettings();
}

void MainWindow::setEditorForTab(int index)
{
    m_loadingDepth++;

    QString sid = (index >= 0 && index < tabWidget->count())
        ? tabWidget->tabBar()->tabData(index).toString()
        : QString();
    currentSnippetId = sid;

    if (sid.isEmpty()) {
        nameEdit->clear();
        descEdit->clear();
        tagsEdit->clear();
        packagesEdit->clear();
        tikzLibrariesEdit->clear();
        loadPreviewForSnippet(QString());
        clearParams();
    } else {
        Snippet s = snippetMgr->loadSnippet(sid);
        if (!s.id.isEmpty()) {
            nameEdit->setText(s.name);
            descEdit->setPlainText(s.description);
            tagsEdit->setText(s.tags.join(", "));
            packagesEdit->setText(s.packages);
            tikzLibrariesEdit->setText(s.tikzLibraries);

            if (!s.templateId.isEmpty()) {
                int ti = templateCombo->findData(s.templateId);
                if (ti >= 0) templateCombo->setCurrentIndex(ti);
            }
            loadPreviewForSnippet(sid);
        }
    }

    m_loadingDepth--;
    parseParams();
}

void MainWindow::onTabChanged(int index)
{
    if (index < 0) {
        currentSnippetId.clear();
        nameEdit->clear();
        descEdit->clear();
        tagsEdit->clear();
        packagesEdit->clear();
        tikzLibrariesEdit->clear();
        clearPdfPreview();
        clearParams();
        return;
    }
    setEditorForTab(index);
}

bool MainWindow::maybeCloseTab(int index)
{
    if (index < 0 || index >= tabWidget->count()) return true;

    QString sid = tabWidget->tabBar()->tabData(index).toString();
    CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(index));
    if (!editor) return true;

    bool hasUnsaved = false;
    if (!sid.isEmpty()) {
        Snippet saved = snippetMgr->loadSnippet(sid);
        if (editor->toPlainText() != saved.code)
            hasUnsaved = true;
    } else {
        if (!editor->toPlainText().trimmed().isEmpty())
            hasUnsaved = true;
    }

    if (hasUnsaved) {
        tabWidget->setCurrentIndex(index);

        QMessageBox msgBox(this);
        msgBox.setWindowTitle(QStringLiteral("未保存的更改"));
        msgBox.setText(QStringLiteral("当前片段 \"%1\" 有未保存的更改。\n\n是否保存？")
                           .arg(tabWidget->tabText(index)));
        msgBox.setIcon(QMessageBox::Warning);
        QPushButton *saveBtn = msgBox.addButton(QStringLiteral("保存"), QMessageBox::AcceptRole);
        QPushButton *discardBtn = msgBox.addButton(QStringLiteral("不保存"), QMessageBox::DestructiveRole);
        QPushButton *cancelBtn = msgBox.addButton(QStringLiteral("取消"), QMessageBox::RejectRole);
        msgBox.setDefaultButton(saveBtn);
        msgBox.exec();

        QAbstractButton *clicked = msgBox.clickedButton();
        if (clicked == saveBtn) {
            saveCurrentSnippet();
            if (sid.isEmpty() && !editor->toPlainText().trimmed().isEmpty()) {
                QDialog dlg(this);
                dlg.setWindowTitle(QStringLiteral("保存新片段"));
                QFormLayout *form = new QFormLayout(&dlg);
                QLineEdit *nameEdit = new QLineEdit;
                QLineEdit *catEdit = new QLineEdit;
                catEdit->setPlaceholderText(QStringLiteral("如: 数学/几何"));
                form->addRow(QStringLiteral("片段名称:"), nameEdit);
                form->addRow(QStringLiteral("分类:"), catEdit);
                QDialogButtonBox *btnBox = new QDialogButtonBox(
                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                form->addRow(btnBox);
                connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
                connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
                if (dlg.exec() == QDialog::Accepted && !nameEdit->text().isEmpty()) {
                    QString id = snippetMgr->createSnippet(nameEdit->text(), catEdit->text());
                    tabWidget->tabBar()->setTabData(index, id);
                    tabWidget->setTabText(index, nameEdit->text());
                    currentSnippetId = id;
                    saveCurrentSnippet();
                } else {
                    return false;
                }
            }
        } else if (clicked == discardBtn) {
        } else {
            return false;
        }
    }

    if (sid == currentSnippetId || currentSnippetId.isEmpty()) {
        currentSnippetId.clear();
    }

    return true;
}

void MainWindow::onTabCloseRequested(int index)
{
    if (!maybeCloseTab(index)) return;

    CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(index));
    tabWidget->removeTab(index);
    if (editor)
        editor->deleteLater();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon && trayIcon->isVisible() && !m_forceQuit) {
        hide();
        event->ignore();
        return;
    }

    m_forceQuit = false;

    QList<int> unsavedTabs;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QString sid = tabWidget->tabBar()->tabData(i).toString();
        CodeEditor *ed = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (!ed) continue;
        bool dirty = false;
        if (!sid.isEmpty()) {
            Snippet saved = snippetMgr->loadSnippet(sid);
            if (ed->toPlainText() != saved.code) dirty = true;
        } else {
            if (!ed->toPlainText().trimmed().isEmpty()) dirty = true;
        }
        if (dirty) unsavedTabs.append(i);
    }

    if (!unsavedTabs.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(QStringLiteral("未保存的更改"));
        msgBox.setText(QStringLiteral("有 %1 个标签页存在未保存的更改。\n\n是否在退出前保存全部？").arg(unsavedTabs.size()));
        msgBox.setIcon(QMessageBox::Warning);
        QPushButton *saveAllBtn = msgBox.addButton(QStringLiteral("保存全部"), QMessageBox::AcceptRole);
        QPushButton *discardAllBtn = msgBox.addButton(QStringLiteral("全部不保存"), QMessageBox::DestructiveRole);
        QPushButton *cancelBtn = msgBox.addButton(QStringLiteral("取消"), QMessageBox::RejectRole);
        msgBox.setDefaultButton(saveAllBtn);
        msgBox.exec();

        QAbstractButton *clicked = msgBox.clickedButton();
        if (clicked == saveAllBtn) {
            for (int idx : unsavedTabs) {
                tabWidget->setCurrentIndex(idx);
                QString sid = tabWidget->tabBar()->tabData(idx).toString();
                currentSnippetId = sid;
                if (sid.isEmpty() && !qobject_cast<CodeEditor*>(tabWidget->widget(idx))->toPlainText().trimmed().isEmpty()) {
                    QDialog dlg(this);
                    dlg.setWindowTitle(QStringLiteral("保存新片段"));
                    QFormLayout *form = new QFormLayout(&dlg);
                    QLineEdit *nmEdit = new QLineEdit;
                    QLineEdit *ctEdit = new QLineEdit;
                    ctEdit->setPlaceholderText(QStringLiteral("如: 数学/几何"));
                    form->addRow(QStringLiteral("片段名称:"), nmEdit);
                    form->addRow(QStringLiteral("分类:"), ctEdit);
                    QDialogButtonBox *btnBox = new QDialogButtonBox(
                        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                    form->addRow(btnBox);
                    connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
                    connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
                    if (dlg.exec() == QDialog::Accepted && !nmEdit->text().isEmpty()) {
                        QString id = snippetMgr->createSnippet(nmEdit->text(), ctEdit->text());
                        tabWidget->tabBar()->setTabData(idx, id);
                        tabWidget->setTabText(idx, nmEdit->text());
                        currentSnippetId = id;
                        saveCurrentSnippet();
                    } else {
                        event->ignore();
                        return;
                    }
                } else {
                    saveCurrentSnippet();
                }
            }
        } else if (clicked == discardAllBtn) {
            clearAllDrafts();
        } else {
            event->ignore();
            return;
        }
    }

    clearAllDrafts();

    if (trayIcon)
        trayIcon->hide();
    event->accept();
    QTimer::singleShot(0, qApp, &QApplication::quit);
}

void MainWindow::setupUI()
{
    setWindowTitle(QStringLiteral("HiTikZ - TikZ 代码合集管理器"));

    QToolBar *toolBar = addToolBar(QStringLiteral("工具栏"));

    QAction *newAct = toolBar->addAction(QStringLiteral("新建片段"));
    QAction *deleteAct = toolBar->addAction(QStringLiteral("删除片段"));

    QToolButton *importExportBtn = new QToolButton;
    importExportBtn->setText(QStringLiteral("导入/导出"));
    importExportBtn->setPopupMode(QToolButton::InstantPopup);
    QMenu *importExportMenu = new QMenu(importExportBtn);
    QAction *importMenuAct = importExportMenu->addAction(QStringLiteral("导入存档"));
    QAction *importTexAct = importExportMenu->addAction(QStringLiteral("导入 .tex 文件"));
    importExportMenu->addSeparator();
    QAction *exportMenuAct = importExportMenu->addAction(QStringLiteral("导出当前"));
    QAction *exportAllMenuAct = importExportMenu->addAction(QStringLiteral("导出全部"));
    importExportMenu->addSeparator();
    QAction *exportTexAct = importExportMenu->addAction(QStringLiteral("导出为 Tex 文档"));
    QAction *exportPdfAct = importExportMenu->addAction(QStringLiteral("导出为 PDF 文档"));
    QAction *exportPngAct = importExportMenu->addAction(QStringLiteral("导出为 PNG 图片"));
    QAction *exportSvgAct = importExportMenu->addAction(QStringLiteral("导出为 SVG 图片"));
    importExportBtn->setMenu(importExportMenu);
    toolBar->addWidget(importExportBtn);

    toolBar->addSeparator();

    QAction *undoAct = toolBar->addAction(QStringLiteral("↩"));
    undoAct->setShortcut(QKeySequence::Undo);
    QAction *redoAct = toolBar->addAction(QStringLiteral("↪"));
    redoAct->setShortcut(QKeySequence::Redo);

    toolBar->addSeparator();

    compileAct = toolBar->addAction(QStringLiteral("编译预览"));
    applyParamsAct = toolBar->addAction(QStringLiteral("应用参数"));
    saveAct = toolBar->addAction(QStringLiteral("保存"));

    toolBar->addSeparator();

    QAction *copyCodeAct = toolBar->addAction(QStringLiteral("复制代码"));
    QAction *copyFullAct = toolBar->addAction(QStringLiteral("复制文档"));
    QAction *copyPngAct = toolBar->addAction(QStringLiteral("复制PNG"));
    QAction *copySvgAct = toolBar->addAction(QStringLiteral("复制SVG"));

    toolBar->addSeparator();

    fitPageAct = toolBar->addAction(QStringLiteral("适应整页"));
    fitPageAct->setCheckable(true);
    fitPageAct->setChecked(true);
    fitWidthAct = toolBar->addAction(QStringLiteral("适应宽度"));
    fitWidthAct->setCheckable(true);
    fitHeightAct = toolBar->addAction(QStringLiteral("适应高度"));
    fitHeightAct->setCheckable(true);
    zoomOutAct = toolBar->addAction(QStringLiteral("−"));
    zoomInAct = toolBar->addAction(QStringLiteral("+"));

    toolBar->addSeparator();

    QAction *settingsAct = toolBar->addAction(QStringLiteral("设置"));

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);

    searchPanel = new SearchPanel(snippetMgr);

    QSplitter *centerSplitter = new QSplitter(Qt::Vertical);
    tabWidget = new QTabWidget;
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);

    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    logPanel = new QPlainTextEdit;
    logPanel->setReadOnly(true);
    logPanel->setMaximumBlockCount(2000);
    logPanel->viewport()->installEventFilter(this);

    centerSplitter->addWidget(tabWidget);
    centerSplitter->addWidget(logPanel);
    centerSplitter->setSizes({600, 100});

    rightPanel = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    pdfPreview = new PdfPreviewWidget;

    QWidget *metadataWidget = new QWidget;
    QVBoxLayout *metaLayout = new QVBoxLayout(metadataWidget);
    metaLayout->setContentsMargins(4, 4, 4, 4);

    QScrollArea *metaScroll = new QScrollArea;
    metaScroll->setWidgetResizable(true);
    metaScroll->setWidget(metadataWidget);
    metaScroll->setMinimumHeight(0);

    QSplitter *rightSplitter = new QSplitter(Qt::Vertical);
    rightSplitter->addWidget(pdfPreview);
    rightSplitter->addWidget(metaScroll);
    rightSplitter->setStretchFactor(0, 3);
    rightSplitter->setStretchFactor(1, 1);
    rightSplitter->setSizes({400, 300});
    rightSplitter->setChildrenCollapsible(false);
    rightLayout->addWidget(rightSplitter);

    connect(fitPageAct, &QAction::triggered, this, [this]() {
        pdfPreview->fitPage();
        updateFitActionStates();
    });
    connect(fitWidthAct, &QAction::triggered, this, [this]() {
        pdfPreview->fitWidth();
        updateFitActionStates();
    });
    connect(fitHeightAct, &QAction::triggered, this, [this]() {
        pdfPreview->fitHeight();
        updateFitActionStates();
    });
    connect(zoomInAct, &QAction::triggered, this, [this]() {
        pdfPreview->zoomIn();
        updateFitActionStates();
    });
    connect(zoomOutAct, &QAction::triggered, this, [this]() {
        pdfPreview->zoomOut();
        updateFitActionStates();
    });

    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText(QStringLiteral("名称"));
    descEdit = new QTextEdit;
    descEdit->setPlaceholderText(QStringLiteral("简介"));
    descEdit->setMaximumHeight(80);

    metaLayout->addWidget(new QLabel(QStringLiteral("名称:")));
    metaLayout->addWidget(nameEdit);
    metaLayout->addWidget(new QLabel(QStringLiteral("简介:")));
    metaLayout->addWidget(descEdit, 1);
    metaLayout->addWidget(new QLabel(QStringLiteral("标签 (逗号分隔):")));
    tagsEdit = new QLineEdit;
    tagsEdit->setPlaceholderText(QStringLiteral("标签1, 标签2, ..."));
    metaLayout->addWidget(tagsEdit);
    metaLayout->addWidget(new QLabel(QStringLiteral("额外宏包 (逗号分隔):")));
    packagesEdit = new QLineEdit;
    packagesEdit->setPlaceholderText(QStringLiteral("如: tikz-3dplot,[european]circuitikz"));
    metaLayout->addWidget(packagesEdit);
    metaLayout->addWidget(new QLabel(QStringLiteral("TikZ库 (逗号分隔):")));
    tikzLibrariesEdit = new QLineEdit;
    tikzLibrariesEdit->setPlaceholderText(QStringLiteral("如: calc,er,angles"));
    metaLayout->addWidget(tikzLibrariesEdit);
    metaLayout->addWidget(new QLabel(QStringLiteral("模板:")));
    templateCombo = new QComboBox;
    metaLayout->addWidget(templateCombo);

    paramsScrollArea = new QScrollArea;
    paramsScrollArea->setWidgetResizable(true);
    paramsScrollArea->setMaximumHeight(150);
    paramsWidget = new QWidget;
    paramsLayout = new QVBoxLayout(paramsWidget);
    paramsLayout->setContentsMargins(0, 0, 0, 0);
    paramsScrollArea->setWidget(paramsWidget);
    metaLayout->addWidget(new QLabel(QStringLiteral("参数:")));
    metaLayout->addWidget(paramsScrollArea);

    connect(newAct, &QAction::triggered, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle(QStringLiteral("新建片段"));
        QFormLayout *form = new QFormLayout(&dlg);
        QLineEdit *nmEdit = new QLineEdit;
        QLineEdit *ctEdit = new QLineEdit;
        ctEdit->setPlaceholderText(QStringLiteral("如: 数学/几何"));
        form->addRow(QStringLiteral("片段名称:"), nmEdit);
        form->addRow(QStringLiteral("分类:"), ctEdit);
        QDialogButtonBox *btnBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        form->addRow(btnBox);
        connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        if (dlg.exec() == QDialog::Accepted && !nmEdit->text().isEmpty()) {
            QString id = snippetMgr->createSnippet(nmEdit->text(), ctEdit->text());
            refreshSearch();
            refreshCategoryTree();
            loadSnippetIntoEditor(id);
        }
    });

    connect(deleteAct, &QAction::triggered, this, [this]() {
        QString cat = searchPanel->currentCategory();
        bool isAllCat = cat.isEmpty();

        if (cat == "__uncategorized__") {
            statusBar()->showMessage(QStringLiteral("未分类是虚拟分类，无法删除"), kStatusBarShortMs);
            return;
        }

        if (currentSnippetId.isEmpty() && !cat.isEmpty() && !isAllCat) {
            int ret = QMessageBox::warning(this, QStringLiteral("删除分类"),
                QStringLiteral("确定删除分类 \"%1\" 及其全部内容吗？").arg(cat),
                QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                int count = snippetMgr->deleteCategory(cat);
                statusBar()->showMessage(QStringLiteral("已删除 %1 个片段").arg(count), kStatusBarShortMs);
                nameEdit->clear();
                tagsEdit->clear();
                packagesEdit->clear();
                tikzLibrariesEdit->clear();
                descEdit->clear();
                pdfPreview->clearDocument();
                while (tabWidget->count() > 0) {
                    QWidget *w = tabWidget->widget(0);
                    tabWidget->removeTab(0);
                    w->deleteLater();
                }
                currentSnippetId.clear();
                refreshSearch();
                refreshCategoryTree();
            }
            return;
        }

        if (currentSnippetId.isEmpty()) return;
        int ret = QMessageBox::question(this, QStringLiteral("确认删除"),
            QStringLiteral("确定要删除此片段吗？"), QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            snippetMgr->deleteSnippet(currentSnippetId);
            int tabIdx = findTabForSnippet(currentSnippetId);
            if (tabIdx >= 0) {
                QWidget *w = tabWidget->widget(tabIdx);
                tabWidget->removeTab(tabIdx);
                w->deleteLater();
            }
            currentSnippetId.clear();
            nameEdit->clear();
            tagsEdit->clear();
            packagesEdit->clear();
            tikzLibrariesEdit->clear();
            descEdit->clear();
            pdfPreview->clearDocument();
            refreshSearch();
            refreshCategoryTree();
        }
    });

    connect(importMenuAct, &QAction::triggered, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this,
            QStringLiteral("导入存档"), "", "TikZ 存档 (*.tar.gz *.zip)");
        if (!filePath.isEmpty()) {
            QStringList imported = snippetMgr->importSnippetsZip(filePath);
            if (imported.isEmpty()) {
                QMessageBox::warning(this, QStringLiteral("导入失败"),
                    QStringLiteral("未能从ZIP文件中导入任何片段。请确认文件格式正确。"));
            } else {
                statusBar()->showMessage(QStringLiteral("成功导入 %1 个片段").arg(imported.size()), kStatusBarLongMs);
                refreshCategoryTree();
                refreshSearch();
                if (!imported.isEmpty()) {
                    loadSnippetIntoEditor(imported.first());
                }
            }
        }
    });

    connect(importTexAct, &QAction::triggered, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this,
            QStringLiteral("导入 .tex 文件"), "", "LaTeX 文件 (*.tex)");
        if (filePath.isEmpty()) return;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, QStringLiteral("导入失败"),
                QStringLiteral("无法读取文件。"));
            return;
        }

        QString content = QString::fromUtf8(file.readAll());
        file.close();

        QString code;
        int docBegin = content.indexOf(QStringLiteral("\\begin{document}"));
        int docEnd = content.indexOf(QStringLiteral("\\end{document}"));
        if (docBegin >= 0 && docEnd > docBegin) {
            int codeStart = content.indexOf('\n', docBegin) + 1;
            code = content.mid(codeStart, docEnd - codeStart).trimmed();
        } else {
            int tikzBegin = content.indexOf(QStringLiteral("\\begin{tikzpicture}"));
            int tikzEnd = content.indexOf(QStringLiteral("\\end{tikzpicture}"));
            if (tikzBegin >= 0 && tikzEnd > tikzBegin) {
                code = content.mid(tikzBegin, tikzEnd + 17 - tikzBegin);
            } else {
                code = content.trimmed();
            }
        }

        if (code.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("导入失败"),
                QStringLiteral("未能从文件中提取 TikZ 代码。"));
            return;
        }

        QString baseName = QFileInfo(filePath).completeBaseName();
        QString id = snippetMgr->createSnippet(baseName, QString());
        Snippet s = snippetMgr->loadSnippet(id);
        s.code = code;
        s.name = baseName;
        snippetMgr->saveSnippet(s);

        refreshCategoryTree();
        refreshSearch();
        loadSnippetIntoEditor(id);
        statusBar()->showMessage(QStringLiteral("已导入: %1").arg(baseName), kStatusBarShortMs);
    });

    connect(exportMenuAct, &QAction::triggered, this, [this]() {
        if (currentSnippetId.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请先选择一个片段"), kStatusBarShortMs);
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出存档"), "", "TikZ 存档 (*.tar.gz)");
        if (!filePath.isEmpty()) {
            if (snippetMgr->exportSnippetZip(currentSnippetId, filePath)) {
                statusBar()->showMessage(QStringLiteral("导出成功"), kStatusBarShortMs);
            } else {
                QMessageBox::warning(this, QStringLiteral("导出失败"),
                    QStringLiteral("无法导出当前片段。"));
            }
        }
    });

    connect(exportAllMenuAct, &QAction::triggered, this, [this]() {
        QList<Snippet> all = snippetMgr->getAllSnippets(true);
        all.append(snippetMgr->getAllPresets(true));
        if (all.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("没有可导出的片段"), kStatusBarShortMs);
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出全部存档"), "all_snippets.tar.gz", "TikZ 存档 (*.tar.gz)");
        if (!filePath.isEmpty()) {
            QStringList allIds;
            for (const Snippet &s : all)
                allIds.append(s.id);
            if (snippetMgr->exportSnippetsZip(allIds, filePath))
                statusBar()->showMessage(QStringLiteral("全部导出成功"), kStatusBarShortMs);
            else
                QMessageBox::warning(this, QStringLiteral("导出失败"),
                    QStringLiteral("无法导出全部片段。"));
        }
    });

    auto copyCode = [this]() {
        CodeEditor *ed = currentEditor();
        if (!ed) return;
        QString code = applyParams(ed->toPlainText());
        code.remove(paramLineRe);
        QApplication::clipboard()->setText(code);
        statusBar()->showMessage(QStringLiteral("代码已复制到剪贴板"), 2000);
    };

    auto copyFullDocument = [this]() {
        if (currentSnippetId.isEmpty()) {
            CodeEditor *ed = currentEditor();
            if (!ed) return;
            QString code = applyParams(ed->toPlainText());
            static QRegularExpression paramLine("^%\\s*@param:.*(\n|\r\n?)?", QRegularExpression::MultilineOption);
            code.remove(paramLine);
            QString fullDoc = compiler->wrapCode(code, QString(), QString(), QString());
            QApplication::clipboard()->setText(fullDoc);
            statusBar()->showMessage(QStringLiteral("完整文档已复制到剪贴板"), 2000);
            return;
        }
        Snippet s = snippetMgr->loadSnippet(currentSnippetId);
        QString code = applyParams(s.code);
        code.remove(paramLineRe);
        QString fullDoc = compiler->wrapCode(code, s.templateId, s.packages, s.tikzLibraries);
        QApplication::clipboard()->setText(fullDoc);
        statusBar()->showMessage(QStringLiteral("完整文档已复制到剪贴板"), 2000);
    };

    connect(undoAct, &QAction::triggered, this, [this]() {
        CodeEditor *ed = currentEditor();
        if (ed) ed->undo();
    });
    connect(redoAct, &QAction::triggered, this, [this]() {
        CodeEditor *ed = currentEditor();
        if (ed) ed->redo();
    });
    connect(exportTexAct, &QAction::triggered, this, [this]() {
        if (currentSnippetId.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请先选择一个片段"), kStatusBarShortMs);
            return;
        }
        Snippet s = snippetMgr->loadSnippet(currentSnippetId);
        QString code = applyParams(s.code);
        code.remove(paramLineRe);
        QString fullDoc = compiler->wrapCode(code, s.templateId, s.packages, s.tikzLibraries);
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出为 .tex 文档"), s.name + ".tex",
            "LaTeX 文档 (*.tex)");
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(fullDoc.toUtf8());
                file.close();
                statusBar()->showMessage(QStringLiteral(".tex 文档导出成功"), kStatusBarShortMs);
            } else {
                QMessageBox::warning(this, QStringLiteral("导出失败"),
                    QStringLiteral("无法写入文件。"));
            }
        }
    });

    connect(exportPdfAct, &QAction::triggered, this, [this]() {
        QString pdfPath;
        if (!currentSnippetId.isEmpty()) {
            pdfPath = snippetDataPath(currentSnippetId) + "/preview.pdf";
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            pdfPath = compiler->pdfPath();
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            statusBar()->showMessage(QStringLiteral("请先编译生成PDF预览"), kStatusBarShortMs);
            return;
        }
        QString defaultName = currentSnippetId.isEmpty() ? "output.pdf"
            : snippetMgr->loadSnippet(currentSnippetId).name + ".pdf";
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出 PDF"), defaultName, "PDF 文件 (*.pdf)");
        if (!filePath.isEmpty()) {
            if (QFile::copy(pdfPath, filePath)) {
                statusBar()->showMessage(QStringLiteral("PDF 导出成功"), kStatusBarShortMs);
            } else {
                QMessageBox::warning(this, QStringLiteral("导出失败"),
                    QStringLiteral("无法复制PDF文件。"));
            }
        }
    });

    connect(exportPngAct, &QAction::triggered, this, [this]() {
        QString pdfPath;
        if (!currentSnippetId.isEmpty()) {
            pdfPath = snippetDataPath(currentSnippetId) + "/preview.pdf";
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            pdfPath = compiler->pdfPath();
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            statusBar()->showMessage(QStringLiteral("请先编译生成PDF预览"), kStatusBarShortMs);
            return;
        }
        QString defaultName = currentSnippetId.isEmpty() ? "output.png"
            : snippetMgr->loadSnippet(currentSnippetId).name + ".png";
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出 PNG 图片"), defaultName, "PNG 图片 (*.png)");
        if (filePath.isEmpty()) return;

        QString outPrefix = QFileInfo(filePath).absolutePath() + "/" + QFileInfo(filePath).completeBaseName();
        QSettings settings("HiTikZ", "TikzManager");
        int pngDpi = settings.value("png/dpi", 300).toInt();
        QProcess pngProc;
        pngProc.start(compiler->pdfToCairoCommand(), QStringList()
            << "-png" << "-r" << QString::number(pngDpi) << "-singlefile" << pdfPath << outPrefix);
        pngProc.waitForFinished(15000);
        if (pngProc.exitCode() == 0 && QFile::exists(outPrefix + ".png")) {
            statusBar()->showMessage(QStringLiteral("PNG 导出成功"), kStatusBarShortMs);
        } else {
            QMessageBox::warning(this, QStringLiteral("导出失败"),
                QStringLiteral("无法生成PNG图片。"));
        }
    });

    connect(exportSvgAct, &QAction::triggered, this, [this]() {
        QString pdfPath;
        if (!currentSnippetId.isEmpty()) {
            pdfPath = snippetDataPath(currentSnippetId) + "/preview.pdf";
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            pdfPath = compiler->pdfPath();
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            statusBar()->showMessage(QStringLiteral("请先编译生成PDF预览"), kStatusBarShortMs);
            return;
        }
        QString defaultName = currentSnippetId.isEmpty() ? "output.svg"
            : snippetMgr->loadSnippet(currentSnippetId).name + ".svg";
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出 SVG 图片"), defaultName, "SVG 图片 (*.svg)");
        if (filePath.isEmpty()) return;

        QString outSvg = QFileInfo(filePath).absolutePath() + "/" + QFileInfo(filePath).completeBaseName() + ".svg";
        QProcess svgProc;
        svgProc.start(compiler->pdfToCairoCommand(), QStringList()
            << "-svg" << pdfPath << outSvg);
        svgProc.waitForFinished(15000);
        if (svgProc.exitCode() == 0 && QFile::exists(outSvg)) {
            statusBar()->showMessage(QStringLiteral("SVG 导出成功"), kStatusBarShortMs);
        } else {
            QMessageBox::warning(this, QStringLiteral("导出失败"),
                QStringLiteral("无法生成SVG图片。"));
        }
    });

    connect(copyCodeAct, &QAction::triggered, this, copyCode);
    connect(copyFullAct, &QAction::triggered, this, copyFullDocument);

    auto copyPngFromCurrentPreview = [this]() {
        if (m_clipboardPngPending) return;
        QString pdfPath;
        if (!currentSnippetId.isEmpty()) {
            pdfPath = snippetDataPath(currentSnippetId) + "/preview.pdf";
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            pdfPath = compiler->pdfPath();
        }
        if (!pdfPath.isEmpty() && QFile::exists(pdfPath)) {
            m_clipboardPngPending = true;
            compiler->convertToPng(pdfPath, 300);
            connect(compiler, &LatexCompiler::conversionFinished, this,
                [this](bool ok, const QString &pngPath) {
                    m_clipboardPngPending = false;
                    if (ok) {
                        QImage img(pngPath);
                        if (!img.isNull()) {
                            QApplication::clipboard()->setImage(img);
                            statusBar()->showMessage(QStringLiteral("PNG已复制到剪贴板"), 2000);
                        }
                    }
                }, Qt::SingleShotConnection);
        } else {
            statusBar()->showMessage(QStringLiteral("请先编译生成PDF预览"), kStatusBarShortMs);
        }
    };

    auto copySvgFromCurrentPreview = [this]() {
        if (m_clipboardSvgPending) return;
        QString pdfPath;
        if (!currentSnippetId.isEmpty()) {
            pdfPath = snippetDataPath(currentSnippetId) + "/preview.pdf";
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            pdfPath = compiler->pdfPath();
        }
        if (!pdfPath.isEmpty() && QFile::exists(pdfPath)) {
            m_clipboardSvgPending = true;
            compiler->convertToSvg(pdfPath);
            connect(compiler, &LatexCompiler::conversionFinished, this,
                [this](bool ok, const QString &svgPath) {
                    m_clipboardSvgPending = false;
                    if (ok) {
                        QFile svgFile(svgPath);
                        if (svgFile.open(QIODevice::ReadOnly)) {
                            QByteArray svgData = svgFile.readAll();
                            svgFile.close();
                            QMimeData *mimeData = new QMimeData;
                            mimeData->setData(QStringLiteral("image/svg+xml"), svgData);
                            QApplication::clipboard()->setMimeData(mimeData);
                            statusBar()->showMessage(QStringLiteral("SVG已复制到剪贴板"), 2000);
                        }
                    }
                }, Qt::SingleShotConnection);
        } else {
            statusBar()->showMessage(QStringLiteral("请先编译生成PDF预览"), kStatusBarShortMs);
        }
    };

    connect(copyPngAct, &QAction::triggered, this, copyPngFromCurrentPreview);
    connect(copySvgAct, &QAction::triggered, this, copySvgFromCurrentPreview);

    connect(settingsAct, &QAction::triggered, this, [this]() {
        SettingsDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            SettingsDialog::applyToCompiler(compiler);
            applyShortcuts();
            applyGlobalHotkey();
            applyAppearanceSettings();
            statusBar()->showMessage(QStringLiteral("设置已保存"), kStatusBarShortMs);
        }
    });

    copyCodeShortcut = new QShortcut(this);
    copyPngShortcut = new QShortcut(this);
    copySvgShortcut = new QShortcut(this);
    compileShortcut = new QShortcut(this);
    applyParamsShortcut = new QShortcut(this);
    saveShortcut = new QShortcut(this);
    closeTabShortcut = new QShortcut(this);
    connect(copyCodeShortcut, &QShortcut::activated, this, copyCode);
    connect(copyPngShortcut, &QShortcut::activated, this, copyPngFromCurrentPreview);
    connect(copySvgShortcut, &QShortcut::activated, this, copySvgFromCurrentPreview);
    connect(compileShortcut, &QShortcut::activated, this, [this]() { compileAct->trigger(); });
    connect(applyParamsShortcut, &QShortcut::activated, this, [this]() { applyParamsAct->trigger(); });
    connect(saveShortcut, &QShortcut::activated, this, [this]() { saveAct->trigger(); });
    connect(closeTabShortcut, &QShortcut::activated, this, [this]() {
        int idx = tabWidget->currentIndex();
        if (idx >= 0) onTabCloseRequested(idx);
    });
    applyShortcuts();

    mainSplitter->addWidget(searchPanel);
    mainSplitter->addWidget(centerSplitter);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({250, 600, 350});

    setCentralWidget(mainSplitter);
    applyAppearanceSettings();
    statusBar()->showMessage(QStringLiteral("就绪"), kStatusBarShortMs);
}

void MainWindow::setupConnections()
{
    connect(searchPanel, &SearchPanel::snippetSelected,
        this, [this](const QString &id) {
            loadSnippetIntoEditor(id);
        });

    connect(snippetMgr, &SnippetManager::categoriesChanged,
        this, &MainWindow::refreshCategoryTree);

    connect(searchPanel, &SearchPanel::batchExportRequested, this, [this](const QStringList &ids) {
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("批量导出"), "selected_snippets.tar.gz", "TikZ 存档 (*.tar.gz)");
        if (!filePath.isEmpty()) {
            if (snippetMgr->exportSnippetsZip(ids, filePath))
                statusBar()->showMessage(QStringLiteral("批量导出成功 (%1 个片段)").arg(ids.size()), kStatusBarShortMs);
            else
                QMessageBox::warning(this, QStringLiteral("导出失败"),
                    QStringLiteral("无法批量导出所选片段。"));
        }
    });

    connect(searchPanel, &SearchPanel::batchCategoryChangeRequested, this, [this](const QStringList &ids) {
        bool ok;
        QString newCat = QInputDialog::getText(this, QStringLiteral("修改分类"),
            QStringLiteral("输入新分类名称:"), QLineEdit::Normal, "", &ok);
        if (!ok) return;
        if (snippetMgr->batchUpdateCategory(ids, newCat)) {
            statusBar()->showMessage(QStringLiteral("已修改 %1 个片段的分类").arg(ids.size()), kStatusBarShortMs);
            searchPanel->refreshSearch();
        }
    });

    connect(searchPanel, &SearchPanel::batchDeleteRequested, this, [this](const QStringList &ids) {
        int ret = QMessageBox::warning(this, QStringLiteral("确认删除"),
            QStringLiteral("确定要删除选中的 %1 个片段吗？\n此操作不可恢复。").arg(ids.size()),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;

        for (const QString &id : ids) {
            int tabIdx = findTabForSnippet(id);
            if (tabIdx >= 0) {
                if (id == currentSnippetId)
                    currentSnippetId.clear();
                QWidget *w = tabWidget->widget(tabIdx);
                tabWidget->removeTab(tabIdx);
                w->deleteLater();
            }
        }

        int count = snippetMgr->batchDeleteSnippets(ids);
        statusBar()->showMessage(QStringLiteral("已删除 %1 个片段").arg(count), kStatusBarShortMs);
        searchPanel->refreshSearch();
    });

    connect(searchPanel, &SearchPanel::exportAllRequested, this, [this]() {
        QList<Snippet> all = snippetMgr->getAllSnippets(true);
        all.append(snippetMgr->getAllPresets(true));
        if (all.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("没有可导出的片段"), kStatusBarShortMs);
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出全部存档"), "all_snippets.tar.gz", "TikZ 存档 (*.tar.gz)");
        if (!filePath.isEmpty()) {
            QStringList allIds;
            for (const Snippet &s : all)
                allIds.append(s.id);
            if (snippetMgr->exportSnippetsZip(allIds, filePath))
                statusBar()->showMessage(QStringLiteral("全部导出成功"), kStatusBarShortMs);
            else
                QMessageBox::warning(this, QStringLiteral("导出失败"),
                    QStringLiteral("无法导出全部片段。"));
        }
    });

    connect(templateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) {
            if (currentSnippetId.isEmpty()) return;
            Snippet s = snippetMgr->loadSnippet(currentSnippetId);
            s.templateId = templateCombo->currentData().toString();
            snippetMgr->saveSnippet(s);
        });

    connect(compileAct, &QAction::triggered, this, [this]() {
        saveCurrentSnippet();
        QString code;
        QString templateId;
        QString snippetId = currentSnippetId;
        QString packages;
        QString tikzLibraries;

        if (currentSnippetId.isEmpty()) {
            CodeEditor *ed = currentEditor();
            if (!ed) return;
            code = applyParams(ed->toPlainText());
            templateId.clear();
            snippetId = "scratch";
        } else {
            Snippet s = snippetMgr->loadSnippet(currentSnippetId);
            code = applyParams(s.code);
            templateId = s.templateId;
            packages = s.packages;
            tikzLibraries = s.tikzLibraries;
        }

        if (code.trimmed().isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请先输入 TikZ 代码"), kStatusBarShortMs);
            return;
        }

        logPanel->clear();
        compiler->compile(code, templateId, snippetId, packages, tikzLibraries);
    });

    connect(applyParamsAct, &QAction::triggered, this, [this]() {
        if (currentSnippetId.isEmpty()) return;
        Snippet s = snippetMgr->loadSnippet(currentSnippetId);
        QString code = applyParams(s.code);
        logPanel->clear();
        compiler->compile(code, s.templateId, currentSnippetId, s.packages, s.tikzLibraries);
    });

    connect(saveAct, &QAction::triggered, this, [this]() {
        saveCurrentSnippet();
        refreshSearch();
    });

    connect(compiler, &LatexCompiler::compilationFinished,
        this, [this](bool success, const QString &pdfPath, const QString &log) {
            if (m_batchGenerating) return;
            QString sid = currentSnippetId.isEmpty() ? QStringLiteral("scratch") : currentSnippetId;
            QString cmd = compiler->xelatexCommand()
                + " -interaction=nonstopmode -halt-on-error -shell-escape "
                + "-output-directory " + compiler->tempDirPath() + sid
                + " " + compiler->tempDirPath() + sid + "/output.tex";
            m_userCodeStartLine = compiler->userCodeStartLine();
            setFormattedLog(success, cmd, log, m_userCodeStartLine);
            if (success) {
                pdfPreview->clearDocument();
                pdfPreview->document()->load(QFileInfo(pdfPath).absoluteFilePath());
    QTimer::singleShot(kZoomApplyDelayMs, pdfPreview, &PdfPreviewWidget::applyZoomPreference);
                savePreviewData(pdfPath, currentSnippetId);
                statusBar()->showMessage(QStringLiteral("编译成功"), kStatusBarShortMs);
            } else {
                pdfPreview->clearDocument();
                jumpToErrorLine(log);
                statusBar()->showMessage(QStringLiteral("编译失败，详见日志"), kStatusBarShortMs);
            }
        });
}

void MainWindow::refreshSearch()
{
    searchPanel->refreshSearch();
}

void MainWindow::refreshCategoryTree()
{
    searchPanel->refreshCategoryTree();
    searchPanel->refreshTagFilter();
}

void MainWindow::loadSnippetIntoEditor(const QString &id)
{
    Snippet s = snippetMgr->loadSnippet(id);
    if (s.id.isEmpty()) return;

    int existingIdx = findTabForSnippet(id);
    if (existingIdx >= 0) {
        tabWidget->setCurrentIndex(existingIdx);
        return;
    }

    m_loadingDepth++;

    currentSnippetId = id;
    createNewTab(id, s.code, s.name.isEmpty() ? id.left(8) : s.name);

    nameEdit->setText(s.name);
    descEdit->setPlainText(s.description);
    tagsEdit->setText(s.tags.join(", "));
    packagesEdit->setText(s.packages);
    tikzLibrariesEdit->setText(s.tikzLibraries);

    if (!s.templateId.isEmpty()) {
        int idx = templateCombo->findData(s.templateId);
        if (idx >= 0) templateCombo->setCurrentIndex(idx);
    }

    loadPreviewForSnippet(id);

    m_loadingDepth--;
    parseParams();
}

void MainWindow::saveCurrentSnippet()
{
    if (currentSnippetId.isEmpty()) return;

    CodeEditor *ed = currentEditor();
    if (!ed) return;

    Snippet s = snippetMgr->loadSnippet(currentSnippetId);
    s.name = nameEdit->text();
    s.description = descEdit->toPlainText();
    s.code = ed->toPlainText();
    s.packages = packagesEdit->text();
    s.tikzLibraries = tikzLibrariesEdit->text();
    QStringList tags;
    for (const QString &tag : tagsEdit->text().split(',')) {
        QString trimmed = tag.trimmed();
        if (!trimmed.isEmpty())
            tags.append(trimmed);
    }
    s.tags = tags;
    snippetMgr->saveSnippet(s);

    int tabIdx = findTabForSnippet(currentSnippetId);
    if (tabIdx >= 0)
        tabWidget->setTabText(tabIdx, s.name.isEmpty() ? currentSnippetId.left(8) : s.name);

    clearDraft();
}

void MainWindow::onCurrentSnippetChanged()
{
    CodeEditor *ed = currentEditor();
    if (ed) {
        int tabIdx = tabWidget->currentIndex();
        if (tabIdx >= 0) {
            QString sid = tabWidget->tabBar()->tabData(tabIdx).toString();
            Snippet s = sid.isEmpty() ? Snippet() : snippetMgr->loadSnippet(sid);
            if (s.code != ed->toPlainText()) {
                QString title = nameEdit->text().isEmpty()
                    ? (sid.isEmpty() ? QStringLiteral("未命名") : sid.left(8))
                    : nameEdit->text();
                if (!title.endsWith(QStringLiteral(" *")))
                    title += QStringLiteral(" *");
                tabWidget->setTabText(tabIdx, title);
            }
        }
    }
    parseParams();
}

void MainWindow::jumpToErrorLine(const QString &logText)
{
    CodeEditor *ed = currentEditor();
    if (!ed) return;

    QRegularExpression re("l\\.(\\d+)");
    QRegularExpressionMatchIterator it = re.globalMatch(logText);
    if (!it.hasNext()) return;

    QRegularExpressionMatch match = it.next();
    int line = match.captured(1).toInt();
    if (line < 1) return;

    int editorLine = line - m_userCodeStartLine + 1;
    if (editorLine < 1) editorLine = 1;

    QTextCursor cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, editorLine - 1);
    ed->setTextCursor(cursor);
    ed->highlightCurrentLine();
    ed->setFocus();
}

void MainWindow::clearParams()
{
    QLayoutItem *child;
    while ((child = paramsLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    currentParams.clear();
}

void MainWindow::parseParams()
{
    clearParams();

    CodeEditor *ed = currentEditor();
    if (!ed) return;

    QString code = ed->toPlainText();
    QRegularExpression re("%\\s*@param:\\s*(\\w+)=(\\S+)");
    QRegularExpressionMatchIterator it = re.globalMatch(code);

    QStringList paramNames;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        ParamInfo param;
        param.name = match.captured(1);
        param.defaultValue = match.captured(2);

        QWidget *row = new QWidget;
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *label = new QLabel(param.name + ":");
        QLineEdit *edit = new QLineEdit(param.defaultValue);
        edit->setMaximumWidth(100);
        rowLayout->addWidget(label);
        rowLayout->addWidget(edit, 1);

        param.edit = edit;
        currentParams.append(param);
        paramsLayout->addWidget(row);
        paramNames.append(param.name);
    }

    paramsLayout->addStretch();
    ed->refreshParamWords(paramNames);
}

QString MainWindow::applyParams(const QString &code)
{
    QString result = code;
    for (const ParamInfo &param : currentParams) {
        result.replace("@@" + param.name + "@@", param.edit->text());
    }
    return result;
}

void MainWindow::clearPdfPreview()
{
    pdfPreview->clearDocument();
}

QString MainWindow::snippetDataPath(const QString &id) const
{
    if (snippetMgr->isPresetId(id))
        return snippetMgr->getPresetPath() + id;
    else
        return snippetMgr->getBasePath() + id;
}

void MainWindow::savePreviewData(const QString &pdfPath, const QString &snippetId)
{
    if (snippetId.isEmpty()) return;

    QString basePath = snippetDataPath(snippetId);
    QString previewPdf = basePath + "/preview.pdf";

    if (QFile::exists(previewPdf))
        QFile::remove(previewPdf);
    if (!QFile::copy(pdfPath, previewPdf)) {
        qWarning() << "Failed to save preview PDF:" << previewPdf;
        return;
    }

    if (m_batchGenerating) {
        QProcess pngProc;
        QStringList args;
        args << "-png" << "-r" << QString::number(kPreviewDpi) << "-singlefile" << pdfPath << (basePath + "/preview");
        pngProc.start("pdftocairo", args);
        pngProc.waitForFinished(10000);
    } else {
        QProcess *pngProc = new QProcess(this);
        connect(pngProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            pngProc, &QProcess::deleteLater);
        QTimer *timeout = new QTimer(pngProc);
        timeout->setSingleShot(true);
        connect(timeout, &QTimer::timeout, pngProc, [pngProc]() {
            if (pngProc->state() != QProcess::NotRunning) {
                pngProc->kill();
            }
        });
        connect(pngProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            timeout, &QTimer::stop);
        timeout->start(15000);
        QStringList args;
        args << "-png" << "-r" << QString::number(kPreviewDpi) << "-singlefile" << pdfPath << (basePath + "/preview");
        pngProc->start("pdftocairo", args);
    }
}

void MainWindow::loadPreviewForSnippet(const QString &id)
{
    if (id.isEmpty()) {
        clearPdfPreview();
        return;
    }

    QString previewPdf = snippetDataPath(id) + "/preview.pdf";
    if (QFile::exists(previewPdf)) {
        pdfPreview->clearDocument();
        pdfPreview->document()->load(QFileInfo(previewPdf).absoluteFilePath());
        QTimer::singleShot(kZoomApplyDelayMs, pdfPreview, &PdfPreviewWidget::applyZoomPreference);
    } else {
        clearPdfPreview();
    }
}

void MainWindow::generateAllPreviews()
{
    QList<Snippet> all = snippetMgr->getAllSnippets(true);
    all.append(snippetMgr->getAllPresets(true));

    if (all.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("没有可生成预览的条目"), kStatusBarShortMs);
        emit batchPreviewFinished();
        return;
    }

    m_previewQueue = all;
    m_previewTotal = all.size();
    m_previewDone = 0;
    m_batchGenerating = true;
    statusBar()->showMessage(QStringLiteral("正在生成所有预览..."), 0);

    m_compileConn = connect(compiler, &LatexCompiler::compilationFinished,
        this, &MainWindow::onBatchPreviewCompiled);

    processNextPreview();
}

void MainWindow::processNextPreview()
{
    if (m_previewQueue.isEmpty()) {
        disconnect(m_compileConn);
        m_batchGenerating = false;
        if (m_batchTimeoutTimer) {
            delete m_batchTimeoutTimer;
            m_batchTimeoutTimer = nullptr;
        }
        statusBar()->showMessage(
            QStringLiteral("预览生成完毕: %1 个条目").arg(m_previewTotal), kStatusBarLongMs);
        refreshSearch();
        emit batchPreviewFinished();
        return;
    }

    Snippet s = m_previewQueue.takeFirst();
    m_currentBatchSnippetId = s.id;
    QString code = resolveParamsFromCode(s.code);

    if (!m_batchTimeoutTimer) {
        m_batchTimeoutTimer = new QTimer(this);
        m_batchTimeoutTimer->setSingleShot(true);
        connect(m_batchTimeoutTimer, &QTimer::timeout, this, [this]() {
            compiler->cancelCompile();
            onBatchPreviewCompiled(false, QString(), QString());
        });
    }
    m_batchTimeoutTimer->start(kBatchCompileTimeoutMs);

    compiler->compile(code, s.templateId, s.id, s.packages, s.tikzLibraries);
}

void MainWindow::onBatchPreviewCompiled(bool success, const QString &pdfPath, const QString &)
{
    if (!m_batchGenerating) return;

    if (m_batchTimeoutTimer)
        m_batchTimeoutTimer->stop();

    m_previewDone++;
    if (success) {
        savePreviewData(pdfPath, m_currentBatchSnippetId);
    }
    statusBar()->showMessage(
        QStringLiteral("生成预览: %1/%2").arg(m_previewDone).arg(m_previewTotal), 0);

    processNextPreview();
}

void MainWindow::refreshTemplateCombo()
{
    const QSignalBlocker blocker(templateCombo);
    templateCombo->clear();
    QString tplDir = SettingsDialog::templateDir();
    QDir d(tplDir);
    QStringList files = d.entryList(QStringList() << "*.tex", QDir::Files);
    for (const QString &f : files) {
        QString id = QFileInfo(f).completeBaseName();
        templateCombo->addItem(id, id);
    }
    if (templateCombo->count() > 0)
        templateCombo->setCurrentIndex(0);
}

void MainWindow::setFormattedLog(bool success, const QString &command, const QString &log, int userCodeStartLine)
{
    logPanel->clear();

    QTextCharFormat cmdFormat;
    cmdFormat.setForeground(QColor(60, 120, 200));
    cmdFormat.setFontWeight(QFont::Bold);

    QTextCharFormat successFormat;
    successFormat.setForeground(QColor(20, 150, 20));
    successFormat.setFontWeight(QFont::Bold);

    QTextCharFormat failureFormat;
    failureFormat.setForeground(QColor(220, 30, 30));
    failureFormat.setFontWeight(QFont::Bold);

    QTextCharFormat errorFormat;
    errorFormat.setForeground(QColor(220, 30, 30));
    errorFormat.setFontWeight(QFont::Bold);

    QTextCharFormat warningFormat;
    warningFormat.setForeground(QColor(200, 140, 0));

    QTextCharFormat lineNumFormat;
    lineNumFormat.setForeground(QColor(180, 60, 60));

    QTextCharFormat defaultFormat;

    logPanel->textCursor().insertText(QStringLiteral("Compile command:\n"), cmdFormat);
    logPanel->textCursor().insertText(command + "\n\n", defaultFormat);

    if (log.isEmpty()) {
        logPanel->textCursor().insertText(
            success ? QStringLiteral("Compilation successful ✓\n") : QStringLiteral("Compilation failed ✗\n"),
            success ? successFormat : failureFormat);
        return;
    }

    const QStringList lines = log.split('\n');
    static const QRegularExpression errorRe("^!\\s");
    static const QRegularExpression warningRe("Warning", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression lineRe("^l\\.(\\d+)");
    static const QRegularExpression outputRe("^Output written on");
    static const QRegularExpression overfullRe("Overfull|Underfull");
    static const QRegularExpression noiseFileRe("^[\\(/].*\\.(sty|cls|def|cfg|fd|aux|tex|map|enc|tfm)");
    static const QRegularExpression noiseBannerRe(
        "^(This is XeTeX|entering extended mode|Transcript written on|No file .*\\.aux)"
    );

    QStringList filtered;
    bool inErrorBlock = false;

    auto adjustLineNum = [&](const QString &line) -> QString {
        QRegularExpressionMatch m = lineRe.match(line.trimmed());
        if (m.hasMatch()) {
            int fullLine = m.captured(1).toInt();
            int editorLine = fullLine - userCodeStartLine + 1;
            if (editorLine < 1) editorLine = 1;
            QString result = line;
            result.replace(QRegularExpression("l\\.\\d+"), "l." + QString::number(editorLine));
            return result;
        }
        return line;
    };

    for (const QString &line : lines) {
        bool isError = errorRe.match(line).hasMatch();
        bool isLineNum = lineRe.match(line.trimmed()).hasMatch();
        bool isWarning = warningRe.match(line).hasMatch() || overfullRe.match(line).hasMatch();
        bool isOutput = outputRe.match(line).hasMatch();
        bool isNoise = noiseBannerRe.match(line).hasMatch()
            || noiseFileRe.match(line.trimmed()).hasMatch();

        if (isError) {
            inErrorBlock = true;
            filtered.append(adjustLineNum(line));
        } else if (isLineNum || isWarning || isOutput) {
            filtered.append(adjustLineNum(line));
            if (isOutput) inErrorBlock = false;
        } else if (success) {
            continue;
        } else if (inErrorBlock && !line.trimmed().isEmpty() && !isNoise) {
            filtered.append(adjustLineNum(line));
        } else if (line.trimmed().isEmpty() || isNoise || line.trimmed() == "?") {
            inErrorBlock = false;
        }
    }

    for (const QString &line : filtered) {
        QTextCharFormat fmt = defaultFormat;
        if (errorRe.match(line).hasMatch()) {
            fmt = errorFormat;
        } else if (lineRe.match(line.trimmed()).hasMatch()) {
            fmt = lineNumFormat;
        } else if (warningRe.match(line).hasMatch() || overfullRe.match(line).hasMatch()) {
            fmt = warningFormat;
        }
        logPanel->textCursor().insertText(line + '\n', fmt);
    }

    logPanel->textCursor().insertText(
        success ? QStringLiteral("\nCompilation successful ✓\n") : QStringLiteral("\nCompilation failed ✗ — see errors above\n"),
        success ? successFormat : failureFormat);

    QTextCursor cursor = logPanel->textCursor();
    cursor.movePosition(QTextCursor::Start);
    logPanel->setTextCursor(cursor);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == logPanel->viewport() && event->type() == QEvent::MouseButtonDblClick) {
        handleLogDoubleClick();
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::handleLogDoubleClick()
{
    CodeEditor *ed = currentEditor();
    if (!ed) return;

    QTextCursor cursor = logPanel->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    QString line = cursor.selectedText().trimmed();

    QRegularExpression re("l\\.(\\d+)");
    QRegularExpressionMatch match = re.match(line);
    if (!match.hasMatch()) return;

    int lineNum = match.captured(1).toInt();
    if (lineNum < 1) return;

    cursor = ed->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNum - 1);
    ed->setTextCursor(cursor);
    ed->highlightCurrentLine();
    ed->setFocus();
}

void MainWindow::checkSystemDependencies()
{
    QStringList missing;

    if (!LatexCompiler::checkXelatexAvailable())
        missing << QStringLiteral("xelatex");

    if (!LatexCompiler::checkPdfToCairoAvailable())
        missing << QStringLiteral("pdftocairo");

    if (missing.isEmpty()) return;

    QString msg = QStringLiteral("以下依赖工具未找到：\n\n");
    for (const QString &m : missing)
        msg += QStringLiteral("  • %1\n").arg(m);
    msg += QStringLiteral("\n这些工具是编译 TikZ 预览和格式转换所必需的。\n\n");
    msg += QStringLiteral("安装方法 (Arch/Manjaro):\n");
    msg += QStringLiteral("  sudo pacman -S texlive-core poppler\n\n");
    msg += QStringLiteral("安装方法 (Debian/Ubuntu):\n");
    msg += QStringLiteral("  sudo apt install texlive-xetex poppler-utils\n\n");
    msg += QStringLiteral("安装方法 (Fedora):\n");
    msg += QStringLiteral("  sudo dnf install texlive-xetex poppler-utils");

    QTimer::singleShot(200, this, [this, msg]() {
        QMessageBox::warning(this, QStringLiteral("缺少依赖"), msg);
    });
}

QString MainWindow::resolveParamsFromCode(const QString &code)
{
    QString result = code;
    QRegularExpression re("%\\s*@param:\\s*(\\w+)=(\\S+)");
    QRegularExpressionMatchIterator it = re.globalMatch(code);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString varName = match.captured(1);
        QString defaultValue = match.captured(2);
        result.replace("@@" + varName + "@@", defaultValue);
    }
    return result;
}

void MainWindow::updateFitActionStates()
{
    int pref = pdfPreview->zoomPreference();
    fitPageAct->setChecked(pref == 0);
    fitWidthAct->setChecked(pref == 1);
    fitHeightAct->setChecked(pref == 2);
}

void MainWindow::applyShortcuts()
{
    QSettings settings("HiTikZ", "TikzManager");

    auto setShortcut = [&](QShortcut *sc, const QString &key) {
        QString val = settings.value(key).toString();
        sc->setKey(val.isEmpty() ? QKeySequence() : QKeySequence(val));
    };

    setShortcut(copyCodeShortcut, "shortcuts/copyCode");
    setShortcut(copyPngShortcut, "shortcuts/copyPng");
    setShortcut(copySvgShortcut, "shortcuts/copySvg");
    setShortcut(compileShortcut, "shortcuts/compile");
    setShortcut(applyParamsShortcut, "shortcuts/applyParams");
    setShortcut(saveShortcut, "shortcuts/save");
    setShortcut(closeTabShortcut, "shortcuts/closeTab");
}

void MainWindow::applyGlobalHotkey()
{
    QSettings settings("HiTikZ", "TikzManager");
    QString keyStr = settings.value("shortcuts/globalHotkey", "Ctrl+Alt+T").toString();

#ifdef HAS_KGLOBALACCEL
    KdeGlobalShortcut *ks = KdeGlobalShortcut::instance();
    ks->disconnect(this);
    if (keyStr.isEmpty()) {
        ks->unregisterShortcut("toggle_window");
        return;
    }

    connect(ks, &KdeGlobalShortcut::activated, this, [this](const QString &id) {
        if (id == "toggle_window") {
            if (isVisible() && !isMinimized()) hide();
            else { show(); raise(); activateWindow(); searchPanel->setFocus(); }
        }
    });
    ks->registerShortcut("toggle_window", QStringLiteral("显示/隐藏窗口"), keyStr);
    return;
#endif

#ifdef HAS_QHOTKEY
    if (globalHotkey) {
        globalHotkey->disconnect(this);
        delete globalHotkey;
        globalHotkey = nullptr;
    }
    if (keyStr.isEmpty()) return;

    QKeySequence ks(keyStr);
    if (ks.isEmpty()) return;

    globalHotkey = new QHotkey(ks, true, this);
    if (globalHotkey->isRegistered()) {
        connect(globalHotkey, &QHotkey::activated, this, [this]() {
            if (isVisible() && !isMinimized()) hide();
            else { show(); raise(); activateWindow(); searchPanel->setFocus(); }
        });
    }
#endif
}

void MainWindow::factoryReset()
{
    QString dataLocation = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    QDir(dataLocation + "/snippets").removeRecursively();
    QDir(dataLocation + "/presets").removeRecursively();
    QDir(dataLocation + "/templates").removeRecursively();
    QDir().mkpath(dataLocation + "/snippets");
    QDir().mkpath(dataLocation + "/presets");
    QDir().mkpath(dataLocation + "/templates");

    QString resourceDir = QStringLiteral(RES_DIR);
    SettingsDialog::ensureTemplatesCopied(resourceDir + "/templates");
    SnippetManager::copyPresetsFromResources(
        resourceDir + "/presets",
        dataLocation + "/presets/");
    SettingsDialog::applyToCompiler(compiler);

    currentSnippetId.clear();
    nameEdit->clear();
    descEdit->clear();
    tagsEdit->clear();
    packagesEdit->clear();
    tikzLibrariesEdit->clear();
    clearPdfPreview();

    while (tabWidget->count() > 0) {
        QWidget *w = tabWidget->widget(0);
        tabWidget->removeTab(0);
        w->deleteLater();
    }

    refreshCategoryTree();
    refreshSearch();
}

void MainWindow::setFitPageChecked(bool checked) { fitPageAct->setChecked(checked); }
void MainWindow::setFitWidthChecked(bool checked) { fitWidthAct->setChecked(checked); }
void MainWindow::setFitHeightChecked(bool checked) { fitHeightAct->setChecked(checked); }

void MainWindow::applyAppearanceSettings()
{
    QSettings settings("HiTikZ", "TikzManager");
    int fontSize = settings.value("editor/fontSize", kDefaultFontSize).toInt();

    QFont editorFont("monospace", fontSize);
    QFont logFont("monospace", qMax(8, fontSize - 1));
    logPanel->setFont(logFont);

    for (int i = 0; i < tabWidget->count(); ++i) {
        CodeEditor *ed = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (ed) {
            ed->setFont(editorFont);
            ed->setTabStopDistance(4 * ed->fontMetrics().horizontalAdvance(' '));
        }
    }
}

void MainWindow::startAutoSave()
{
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setInterval(kAutoSaveIntervalMs);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::performAutoSave);
    autoSaveTimer->start();
}

void MainWindow::performAutoSave()
{
    for (int i = 0; i < tabWidget->count(); ++i) {
        CodeEditor *ed = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (!ed || ed->toPlainText().trimmed().isEmpty())
            continue;

        QString sid = tabWidget->tabBar()->tabData(i).toString();
        QString draftDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/drafts/";
        QDir().mkpath(draftDir);

        QString draftPath = draftDir + (sid.isEmpty() ? "scratch" : sid) + ".json";

        QJsonObject obj;
        obj["snippetId"] = sid;
        obj["code"] = ed->toPlainText();
        obj["name"] = tabWidget->tabText(i);

        if (sid.isEmpty()) {
            obj["description"] = QString();
            obj["tags"] = QString();
            obj["packages"] = QString();
            obj["tikzLibraries"] = QString();
            obj["templateId"] = QString();
        } else {
            Snippet s = snippetMgr->loadSnippet(sid);
            obj["description"] = s.description;
            obj["tags"] = s.tags.join(", ");
            obj["packages"] = s.packages;
            obj["tikzLibraries"] = s.tikzLibraries;
            obj["templateId"] = s.templateId;
        }

        QFile file(draftPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(QJsonDocument(obj).toJson());
            file.close();
        }
    }
}

void MainWindow::clearDraft()
{
    QString draftDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/drafts/";
    QString draftPath = draftDir + (currentSnippetId.isEmpty() ? "scratch" : currentSnippetId) + ".json";
    QFile::remove(draftPath);
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
    }

    for (const DraftInfo &draft : drafts)
        QFile::remove(draft.filePath);

    if (!recoveredIndices.isEmpty()) {
        refreshCategoryTree();
        refreshSearch();
    }
}
