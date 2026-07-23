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
#include "comma_list_completer.h"
#include "tikz_words.h"
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

#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define STRINGIFY_IMPL(x) #x
#define RES_DIR STRINGIFY(RESOURCE_DIR)
#define INSTALL_RES_DIR STRINGIFY(INSTALL_RESOURCE_DIR)

// Locate the bundled resources (presets/templates). Prefer the installed
// location (set at configure time); fall back to the in-tree source directory
// so the program also runs directly from the build tree during development.
static QString resolveResourceDir()
{
    const QString installDir = QStringLiteral(INSTALL_RES_DIR);
    if (!installDir.isEmpty() && QDir(installDir).exists())
        return installDir;
    return QStringLiteral(RES_DIR);
}

#include "mainwindow_internal.h"

static const QRegularExpression paramLineRe(QStringLiteral("^%\\s*@param:.*(\n|\r\n?)?"), QRegularExpression::MultilineOption);

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QString resourceDir = resolveResourceDir();
    SettingsDialog::ensureTemplatesCopied(resourceDir + "/templates");

    // Only copy presets on first run (tracked via QSettings flag)
    QString presetDest = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/presets/";
    QSettings settings("HiTikZ", "TikzManager");
    if (!settings.value("presets/installed", false).toBool()) {
        SnippetManager::copyPresetsFromResources(resourceDir + "/presets", presetDest);
        settings.setValue("presets/installed", true);
    }

    snippetMgr = new SnippetManager(this);
    compiler = new LatexCompiler(this);
    SettingsDialog::applyToCompiler(compiler);

    setupUI();
    setupConnections();
    refreshTemplateCombo();
    refreshCategoryTree();
    refreshSearch();

    restoreWindowGeometry();

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon::fromTheme(QStringLiteral("hitikz"),
                                       QIcon::fromTheme(QStringLiteral("applications-graphics"))));
    trayIcon->setToolTip(QStringLiteral("HiTikZ - TikZ 代码管理器"));

    trayMenu = new QMenu(this);
    QAction *showAct = trayMenu->addAction(QStringLiteral("显示窗口"));
    QAction *hideAct = trayMenu->addAction(QStringLiteral("隐藏到托盘"));
    trayMenu->addSeparator();
    QAction *quitAct = trayMenu->addAction(QStringLiteral("退出"));

    connect(showAct, &QAction::triggered, this, [this]() {
        restoreWindowGeometry();
        show();
        raise();
        activateWindow();
        searchPanel->setFocus();
    });
    connect(hideAct, &QAction::triggered, this, [this]() {
        saveWindowGeometry();
        hide();
    });
    connect(quitAct, &QAction::triggered, this, [this]() {
        m_forceQuit = true;
        close();
    });
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger) {
            if (isVisible() && !isMinimized()) {
                saveWindowGeometry();
                hide();
            } else {
                restoreWindowGeometry();
                show();
                raise();
                activateWindow();
                searchPanel->setFocus();
            }
        }
    });

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    // Startup dialogs (dependency warnings, draft recovery) are deferred while
    // the window is hidden: with --hidden autostart nothing may pop up at
    // login — these run on the first showEvent() instead.
    QTimer::singleShot(100, this, [this]() {
        if (isVisible())
            checkSystemDependencies();
        else
            m_pendingDependencyCheck = true;
    });

    applyGlobalHotkey();

    m_parseParamsTimer = new QTimer(this);
    m_parseParamsTimer->setSingleShot(true);
    m_parseParamsTimer->setInterval(300);
    connect(m_parseParamsTimer, &QTimer::timeout, this, &MainWindow::performParseParams);

    startAutoSave();
    QTimer::singleShot(300, this, [this]() {
        if (isVisible())
            recoverDrafts();
        else
            m_pendingDraftRecovery = true;
    });
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (m_pendingDependencyCheck) {
        m_pendingDependencyCheck = false;
        QTimer::singleShot(100, this, [this]() {
            checkSystemDependencies();
        });
    }
    if (m_pendingDraftRecovery) {
        QTimer::singleShot(300, this, [this]() {
            if (m_pendingDraftRecovery) {
                m_pendingDraftRecovery = false;
                recoverDrafts();
            }
        });
    }
}

void MainWindow::centerOnScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect screenGeometry = screen->availableGeometry();

    int w = qMin(width(), screenGeometry.width() - 20);
    int h = qMin(height(), screenGeometry.height() - 20);
    if (w != width() || h != height())
        resize(w, h);

    int x = screenGeometry.x() + (screenGeometry.width() - w) / 2;
    int y = screenGeometry.y() + (screenGeometry.height() - h) / 2;

    move(x, y);
}

void MainWindow::saveWindowGeometry()
{
    QSettings settings("HiTikZ", "TikzManager");
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());
}

void MainWindow::restoreWindowGeometry()
{
    QSettings settings("HiTikZ", "TikzManager");
    QByteArray geometry = settings.value("window/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
        restoreState(settings.value("window/state").toByteArray());
    } else {
        resize(1400, 800);
        centerOnScreen();
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon && trayIcon->isVisible() && !m_forceQuit) {
        saveWindowGeometry();
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
        if (isSnippetDirty(sid, ed)) unsavedTabs.append(i);
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

    // Drafts from a previous crashed session that the user has not been shown
    // yet (hidden autostart, quit from tray before the first show) must
    // survive this shutdown so recovery can still be offered next start.
    if (!m_pendingDraftRecovery)
        clearAllDrafts();
    saveWindowGeometry();

    if (trayIcon)
        trayIcon->hide();
    event->accept();
    QTimer::singleShot(0, qApp, &QApplication::quit);
}

void MainWindow::setupUI()
{
    setWindowTitle(QStringLiteral("HiTikZ - TikZ 代码合集管理器"));

    QToolBar *toolBar = addToolBar(QStringLiteral("工具栏"));
    toolBar->setObjectName(QStringLiteral("mainToolBar"));

    QAction *newAct = toolBar->addAction(QStringLiteral("新建片段"));
    QAction *deleteAct = toolBar->addAction(QStringLiteral("删除片段"));
    duplicateAct = toolBar->addAction(QStringLiteral("复制片段"));

    QToolButton *importExportBtn = new QToolButton;
    importExportBtn->setText(QStringLiteral("导入/导出"));
    importExportBtn->setPopupMode(QToolButton::InstantPopup);
    QMenu *importExportMenu = new QMenu(importExportBtn);
    QAction *importMenuAct = importExportMenu->addAction(QStringLiteral("导入存档"));
    QAction *importTexAct = importExportMenu->addAction(QStringLiteral("导入 .tex 文件"));
    QAction *importClipAct = importExportMenu->addAction(QStringLiteral("从剪贴板导入"));
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

    // Theme icons, not emoji glyphs: codepoints like ↩/↪/⛔ are rendered by
    // color-emoji fonts (e.g. Twemoji) on many systems, and color glyphs are
    // NOT tinted by the disabled palette — a disabled button would not look
    // greyed out. Theme icons render a proper disabled variant; the text is
    // the fallback where the icon theme lacks them.
    undoAct = toolBar->addAction(QIcon::fromTheme(QStringLiteral("edit-undo")),
                                 QStringLiteral("撤销"));
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setToolTip(QStringLiteral("撤销"));
    undoAct->setEnabled(false);
    redoAct = toolBar->addAction(QIcon::fromTheme(QStringLiteral("edit-redo")),
                                 QStringLiteral("重做"));
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setToolTip(QStringLiteral("重做"));
    redoAct->setEnabled(false);

    toolBar->addSeparator();

    compileAct = toolBar->addAction(QStringLiteral("编译预览"));
    applyParamsAct = toolBar->addAction(QStringLiteral("应用参数"));
    saveAct = toolBar->addAction(QStringLiteral("保存"));

    forceStopAct = toolBar->addAction(QIcon::fromTheme(QStringLiteral("process-stop")),
                                      QStringLiteral("强制结束"));
    forceStopAct->setEnabled(false);

    toolBar->addSeparator();

    QAction *copyCodeAct = toolBar->addAction(QStringLiteral("复制代码"));
    QAction *copyFullAct = toolBar->addAction(QStringLiteral("复制文档"));
    QAction *copyPngAct = toolBar->addAction(QStringLiteral("复制PNG"));
    QAction *copySvgAct = toolBar->addAction(QStringLiteral("复制SVG"));
    openPdfExternalAct = toolBar->addAction(QStringLiteral("外部PDF"));

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
    nameEdit->setObjectName(QStringLiteral("metaNameEdit"));
    nameEdit->setPlaceholderText(QStringLiteral("名称"));
    descEdit = new QTextEdit;
    descEdit->setObjectName(QStringLiteral("metaDescEdit"));
    descEdit->setPlaceholderText(QStringLiteral("简介"));
    descEdit->setMaximumHeight(80);

    metaLayout->addWidget(new QLabel(QStringLiteral("名称:")));
    metaLayout->addWidget(nameEdit);
    metaLayout->addWidget(new QLabel(QStringLiteral("简介:")));
    metaLayout->addWidget(descEdit, 1);
    metaLayout->addWidget(new QLabel(QStringLiteral("标签 (逗号分隔):")));
    tagsEdit = new QLineEdit;
    tagsEdit->setObjectName(QStringLiteral("metaTagsEdit"));
    tagsEdit->setPlaceholderText(QStringLiteral("标签1, 标签2, ..."));
    metaLayout->addWidget(tagsEdit);
    metaLayout->addWidget(new QLabel(QStringLiteral("额外宏包 (逗号分隔):")));
    packagesEdit = new QLineEdit;
    packagesEdit->setObjectName(QStringLiteral("metaPackagesEdit"));
    packagesEdit->setPlaceholderText(QStringLiteral("如: tikz-3dplot,[european]circuitikz"));
    {
        // Amber for LaTeX packages, blue for TikZ libraries: the accent colors
        // make the two popups (and their items) recognizable at a glance.
        auto *pkgCompleter = new CommaListCompleter(TikzWords::latexPackages(), this,
                                                    QColor(0xe6, 0x7e, 0x22));
        packagesEdit->setCompleter(pkgCompleter);
    }
    metaLayout->addWidget(packagesEdit);
    metaLayout->addWidget(new QLabel(QStringLiteral("TikZ库 (逗号分隔):")));
    tikzLibrariesEdit = new QLineEdit;
    tikzLibrariesEdit->setObjectName(QStringLiteral("metaTikzLibrariesEdit"));
    tikzLibrariesEdit->setPlaceholderText(QStringLiteral("如: calc,er,angles"));
    {
        auto *libCompleter = new CommaListCompleter(TikzWords::tikzLibraries(), this,
                                                    QColor(0x2e, 0x86, 0xc1));
        tikzLibrariesEdit->setCompleter(libCompleter);
    }
    metaLayout->addWidget(tikzLibrariesEdit);
    metaLayout->addWidget(new QLabel(QStringLiteral("模板:")));
    templateCombo = new QComboBox;
    metaLayout->addWidget(templateCombo);

    // Track metadata edits so the '*' tab suffix and tab-switch isolation work.
    connect(nameEdit, &QLineEdit::textChanged, this, &MainWindow::updateTabUiState);
    connect(descEdit, &QTextEdit::textChanged, this, &MainWindow::updateTabUiState);
    connect(tagsEdit, &QLineEdit::textChanged, this, &MainWindow::updateTabUiState);
    connect(packagesEdit, &QLineEdit::textChanged, this, &MainWindow::updateTabUiState);
    connect(tikzLibrariesEdit, &QLineEdit::textChanged, this, &MainWindow::updateTabUiState);
    connect(templateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::updateTabUiState);

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

        QComboBox *tplCombo = new QComboBox;
        QString tplDir = SettingsDialog::templateDir();
        QDir d(tplDir);
        QStringList tplFiles = d.entryList(QStringList() << "*.tex", QDir::Files);
        for (const QString &f : tplFiles) {
            QString tid = QFileInfo(f).completeBaseName();
            tplCombo->addItem(tid, tid);
        }
        int defaultTplIdx = tplCombo->findData(QStringLiteral("default_math"));
        if (defaultTplIdx >= 0)
            tplCombo->setCurrentIndex(defaultTplIdx);

        form->addRow(QStringLiteral("片段名称:"), nmEdit);
        form->addRow(QStringLiteral("分类:"), ctEdit);
        form->addRow(QStringLiteral("模板:"), tplCombo);
        QDialogButtonBox *btnBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        form->addRow(btnBox);
        connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        if (dlg.exec() == QDialog::Accepted && !nmEdit->text().isEmpty()) {
            QString id = snippetMgr->createSnippet(nmEdit->text(), ctEdit->text());
            Snippet s = snippetMgr->loadSnippet(id);
            s.templateId = tplCombo->currentData().toString();
            snippetMgr->saveSnippet(s);
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
                    if (!maybeCloseTab(0)) {
                        int next = tabWidget->count() - 1;
                        tabWidget->setCurrentIndex(next >= 0 ? next : 0);
                        break;
                    }
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
            QString idToDelete = currentSnippetId;
            int tabIdx = findTabForSnippet(idToDelete);
            if (tabIdx >= 0 && !maybeCloseTab(tabIdx))
                return;
            snippetMgr->deleteSnippet(idToDelete);
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

        // Split into preamble and body
        QString preamble;
        QString code;
        int docBegin = content.indexOf(QStringLiteral("\\begin{document}"));
        int docEnd = content.indexOf(QStringLiteral("\\end{document}"));
        if (docBegin >= 0 && docEnd > docBegin) {
            preamble = content.left(docBegin);
            int codeStart = content.indexOf('\n', docBegin) + 1;
            code = content.mid(codeStart, docEnd - codeStart).trimmed();
        } else {
            preamble = content;
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

        // Parse \usepackage{...} and \usepackage[...]{...} from preamble
        QStringList packages;
        QRegularExpression usepkgRe(QStringLiteral("\\\\usepackage(?:\\[([^\\]]*)\\])?\\{([^}]*)\\}"));
        QRegularExpressionMatchIterator pkgIt = usepkgRe.globalMatch(preamble);
        while (pkgIt.hasNext()) {
            QRegularExpressionMatch m = pkgIt.next();
            QString options = m.captured(1);
            QString pkgList = m.captured(2);
            QStringList pkgs = pkgList.split(',');
            for (const QString &pkg : pkgs) {
                QString trimmed = pkg.trimmed();
                if (trimmed.isEmpty()) continue;
                if (!options.isEmpty())
                    trimmed = "[" + options + "]" + trimmed;
                packages.append(trimmed);
            }
        }

        // Parse \usetikzlibrary{...} from preamble
        QStringList libraries;
        QRegularExpression uselibRe(QStringLiteral("\\\\usetikzlibrary\\{([^}]*)\\}"));
        QRegularExpressionMatchIterator libIt = uselibRe.globalMatch(preamble);
        while (libIt.hasNext()) {
            QRegularExpressionMatch m = libIt.next();
            QString libList = m.captured(1);
            QStringList libs = libList.split(',');
            for (const QString &lib : libs) {
                QString trimmed = lib.trimmed();
                if (!trimmed.isEmpty())
                    libraries.append(trimmed);
            }
        }

        // Parse \newcommand and \NewDocumentCommand from preamble
        QString preambleCustomCmds;
        QString preambleCleaned;
        preambleCustomCmds = LatexCompiler::extractCustomCommands(preamble, preambleCleaned);
        if (!preambleCustomCmds.trimmed().isEmpty()) {
            preambleCustomCmds = preambleCustomCmds.trimmed() + "\n";
        }

        // Also extract custom commands from the body that appear before tikzpicture
        QString bodyCustomCmds;
        QString bodyCleaned;
        bodyCustomCmds = LatexCompiler::extractCustomCommands(code, bodyCleaned);
        if (!bodyCustomCmds.trimmed().isEmpty()) {
            bodyCustomCmds = bodyCustomCmds.trimmed() + "\n";
        }
        code = bodyCleaned;

        // Combine custom commands (preamble first, then body)
        QString allCustomCmds = (preambleCustomCmds + bodyCustomCmds).trimmed();

        // Build final code: custom commands at top, then tikz code below
        if (!allCustomCmds.isEmpty()) {
            code = allCustomCmds + "\n\n" + code;
        }

        QString baseName = QFileInfo(filePath).completeBaseName();
        QString id = snippetMgr->createSnippet(baseName, QString());
        Snippet s = snippetMgr->loadSnippet(id);
        s.code = code;
        s.name = baseName;
        s.packages = packages.join(", ");
        s.tikzLibraries = libraries.join(", ");
        {
            QString allPkgs = packages.join(" ").toLower();
            if (allPkgs.contains("circuitikz"))
                s.templateId = "default_circuit";
            else
                s.templateId = "default_math";
        }
        snippetMgr->saveSnippet(s);

        refreshCategoryTree();
        refreshSearch();
        loadSnippetIntoEditor(id);
        statusBar()->showMessage(QStringLiteral("已导入: %1").arg(baseName), kStatusBarShortMs);
    });

    connect(importClipAct, &QAction::triggered, this, [this]() {
        QString content = QApplication::clipboard()->text();
        if (content.trimmed().isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("导入失败"),
                QStringLiteral("剪贴板中没有文本内容。"));
            return;
        }

        QString trimmed = content.trimmed();
        QString clipName;
        int firstLineEnd = trimmed.indexOf('\n');
        if (firstLineEnd > 0) {
            clipName = trimmed.left(firstLineEnd).trimmed();
            if (clipName.startsWith('%'))
                clipName = clipName.mid(1).trimmed();
            if (clipName.length() > 40)
                clipName = clipName.left(40);
        }
        if (clipName.isEmpty()) {
            clipName = QStringLiteral("来自剪贴板");
        }

        QString preamble;
        QString code;
        int docBegin = content.indexOf(QStringLiteral("\\begin{document}"));
        int docEnd = content.indexOf(QStringLiteral("\\end{document}"));
        if (docBegin >= 0 && docEnd > docBegin) {
            preamble = content.left(docBegin);
            int codeStart = content.indexOf('\n', docBegin) + 1;
            code = content.mid(codeStart, docEnd - codeStart).trimmed();
        } else {
            preamble = content;
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
                QStringLiteral("未能从剪贴板内容中提取 TikZ 代码。"));
            return;
        }

        QStringList packages;
        QRegularExpression usepkgRe(QStringLiteral("\\\\usepackage(?:\\[([^\\]]*)\\])?\\{([^}]*)\\}"));
        QRegularExpressionMatchIterator pkgIt = usepkgRe.globalMatch(preamble);
        while (pkgIt.hasNext()) {
            QRegularExpressionMatch m = pkgIt.next();
            QString options = m.captured(1);
            QString pkgList = m.captured(2);
            QStringList pkgs = pkgList.split(',');
            for (const QString &pkg : pkgs) {
                QString t = pkg.trimmed();
                if (t.isEmpty()) continue;
                if (!options.isEmpty()) t = "[" + options + "]" + t;
                packages.append(t);
            }
        }

        QStringList libraries;
        QRegularExpression uselibRe(QStringLiteral("\\\\usetikzlibrary\\{([^}]*)\\}"));
        QRegularExpressionMatchIterator libIt = uselibRe.globalMatch(preamble);
        while (libIt.hasNext()) {
            QRegularExpressionMatch m = libIt.next();
            QStringList libs = m.captured(1).split(',');
            for (const QString &lib : libs) {
                QString t = lib.trimmed();
                if (!t.isEmpty()) libraries.append(t);
            }
        }

        QString preambleCustomCmds;
        QString preambleCleaned;
        preambleCustomCmds = LatexCompiler::extractCustomCommands(preamble, preambleCleaned);
        if (!preambleCustomCmds.trimmed().isEmpty()) {
            preambleCustomCmds = preambleCustomCmds.trimmed() + "\n";
        }

        QString bodyCustomCmds;
        QString bodyCleaned;
        bodyCustomCmds = LatexCompiler::extractCustomCommands(code, bodyCleaned);
        if (!bodyCustomCmds.trimmed().isEmpty()) {
            bodyCustomCmds = bodyCustomCmds.trimmed() + "\n";
        }
        code = bodyCleaned;

        QString allCustomCmds = (preambleCustomCmds + bodyCustomCmds).trimmed();
        if (!allCustomCmds.isEmpty()) {
            code = allCustomCmds + "\n\n" + code;
        }

        QString id = snippetMgr->createSnippet(clipName, QString());
        Snippet s = snippetMgr->loadSnippet(id);
        s.code = code;
        s.name = clipName;
        s.packages = packages.join(", ");
        s.tikzLibraries = libraries.join(", ");
        {
            QString allPkgs = packages.join(" ").toLower();
            if (allPkgs.contains("circuitikz"))
                s.templateId = "default_circuit";
            else
                s.templateId = "default_math";
        }
        snippetMgr->saveSnippet(s);

        refreshCategoryTree();
        refreshSearch();
        loadSnippetIntoEditor(id);
        statusBar()->showMessage(QStringLiteral("已从剪贴板导入: %1").arg(clipName), kStatusBarShortMs);
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
            QString cleanedCode;
            QString customCmds = LatexCompiler::extractCustomCommands(code, cleanedCode);
            QString fullDoc = compiler->wrapCode(cleanedCode, QString(), QString(), QString(), customCmds);
            QApplication::clipboard()->setText(fullDoc);
            statusBar()->showMessage(QStringLiteral("完整文档已复制到剪贴板"), 2000);
            return;
        }
        Snippet s = snippetMgr->loadSnippet(currentSnippetId);
        QString code = applyParams(s.code);
        code.remove(paramLineRe);
        QString cleanedCode;
        QString customCmds = LatexCompiler::extractCustomCommands(code, cleanedCode);
        QString fullDoc = compiler->wrapCode(cleanedCode, s.templateId, s.packages, s.tikzLibraries, customCmds);
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
        QString cleanedCode;
        QString customCmds = LatexCompiler::extractCustomCommands(code, cleanedCode);
        QString fullDoc = compiler->wrapCode(cleanedCode, s.templateId, s.packages, s.tikzLibraries, customCmds);
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

        if (compiler->convertToSvgBlocking(pdfPath, filePath)) {
            statusBar()->showMessage(QStringLiteral("SVG 导出成功"), kStatusBarShortMs);
        } else {
            QMessageBox::warning(this, QStringLiteral("导出失败"),
                QStringLiteral("无法生成SVG图片。"));
        }
    });

    connect(copyCodeAct, &QAction::triggered, this, copyCode);
    connect(copyFullAct, &QAction::triggered, this, copyFullDocument);

    auto copyPngFromCurrentPreview = [this]() {
        if (m_clipboardPngPending || m_clipboardSvgPending) return;
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
        if (m_clipboardSvgPending || m_clipboardPngPending) return;
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
                            mimeData->setData(QStringLiteral("text/plain"), svgData);
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

    connect(openPdfExternalAct, &QAction::triggered, this, [this]() {
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
        QSettings settings("HiTikZ", "TikzManager");
        QString viewerCmd = settings.value("tools/pdfViewer", "").toString().trimmed();
        if (viewerCmd.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请在设置中配置外部PDF查看器命令"), kStatusBarLongMs);
            return;
        }
        QStringList parts = QProcess::splitCommand(viewerCmd);
        if (parts.isEmpty()) return;
        QString program = parts.takeFirst();
        parts.append(pdfPath);
        if (!QProcess::startDetached(program, parts)) {
            statusBar()->showMessage(QStringLiteral("启动外部PDF查看器失败"), kStatusBarLongMs);
        }
    });

    connect(settingsAct, &QAction::triggered, this, [this]() {
        SettingsDialog dlg(this);
        dlg.setSnippetManager(snippetMgr);
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

    m_compileStatusLabel = new QLabel;
    m_compileStatusLabel->setVisible(false);
    m_compileStatusTimer = new QTimer(this);
    m_compileStatusTimer->setSingleShot(true);
    connect(m_compileStatusTimer, &QTimer::timeout, this, [this]() {
        m_compileStatusLabel->setVisible(false);
    });
    statusBar()->addWidget(m_compileStatusLabel);
    statusBar()->showMessage(QStringLiteral("就绪"), kStatusBarShortMs);
}

void MainWindow::setupConnections()
{
    connect(searchPanel, &SearchPanel::snippetSelected,
        this, [this](const QString &id) {
            loadSnippetIntoEditor(id);
        });

    connect(searchPanel, &SearchPanel::copySnippetRequested,
        this, [this](const QString &id) {
            Snippet orig = snippetMgr->loadSnippet(id);
            if (orig.id.isEmpty()) return;
            QString baseName = orig.name;
            QString newId = snippetMgr->createSnippet(baseName, orig.category);
            Snippet dup = snippetMgr->loadSnippet(newId);
            dup.code = orig.code;
            dup.description = orig.description;
            dup.tags = orig.tags;
            dup.templateId = orig.templateId;
            dup.packages = orig.packages;
            dup.tikzLibraries = orig.tikzLibraries;
            dup.compileCommand = orig.compileCommand;
            snippetMgr->saveSnippet(dup);
            refreshSearch();
            refreshCategoryTree();
            statusBar()->showMessage(QStringLiteral("片段已复制"), kStatusBarShortMs);
        });

    connect(searchPanel, &SearchPanel::addSnippetRequested,
        this, [this](const QString &category) {
            QDialog dlg(this);
            dlg.setWindowTitle(QStringLiteral("新建片段"));
            QFormLayout *form = new QFormLayout(&dlg);
            QLineEdit *nmEdit = new QLineEdit;
            QLineEdit *ctEdit = new QLineEdit;
            ctEdit->setText(category);
            ctEdit->setPlaceholderText(QStringLiteral("如: 数学/几何"));

            QComboBox *tplCombo = new QComboBox;
            QString tplDir = SettingsDialog::templateDir();
            QDir d(tplDir);
            QStringList tplFiles = d.entryList(QStringList() << "*.tex", QDir::Files);
            for (const QString &f : tplFiles) {
                QString tid = QFileInfo(f).completeBaseName();
                tplCombo->addItem(tid, tid);
            }
            int defaultTplIdx = tplCombo->findData(QStringLiteral("default_math"));
            if (defaultTplIdx >= 0)
                tplCombo->setCurrentIndex(defaultTplIdx);

            form->addRow(QStringLiteral("片段名称:"), nmEdit);
            form->addRow(QStringLiteral("分类:"), ctEdit);
            form->addRow(QStringLiteral("模板:"), tplCombo);
            QDialogButtonBox *btnBox = new QDialogButtonBox(
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            form->addRow(btnBox);
            connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
            connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            if (dlg.exec() == QDialog::Accepted && !nmEdit->text().isEmpty()) {
                QString id = snippetMgr->createSnippet(nmEdit->text(), ctEdit->text());
                Snippet s = snippetMgr->loadSnippet(id);
                s.templateId = tplCombo->currentData().toString();
                snippetMgr->saveSnippet(s);
                refreshSearch();
                refreshCategoryTree();
                loadSnippetIntoEditor(id);
            }
        });

    connect(snippetMgr, &SnippetManager::categoriesChanged,
        this, &MainWindow::refreshCategoryTree);

    connect(snippetMgr, &SnippetManager::snippetModified,
        this, [this](const QString &id) {
            if (id.isEmpty() || id != currentSnippetId) return;
            int tabIdx = findTabForSnippet(id);
            if (tabIdx < 0) return;
            Snippet s = snippetMgr->loadSnippet(id);
            if (s.id.isEmpty()) return;
            m_loadingDepth++;
            nameEdit->setText(s.name);
            descEdit->setPlainText(s.description);
            tagsEdit->setText(s.tags.join(", "));
            packagesEdit->setText(s.packages);
            tikzLibrariesEdit->setText(s.tikzLibraries);
            if (!s.templateId.isEmpty()) {
                int ti = templateCombo->findData(s.templateId);
                if (ti >= 0) templateCombo->setCurrentIndex(ti);
            }
            updateTabTitle(tabIdx, s.name.isEmpty() ? id.left(8) : s.name);
            m_loadingDepth--;
        });

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
            searchPanel->refreshTagFilter();
            searchPanel->refreshSearch();
        }
    });

    connect(searchPanel, &SearchPanel::batchDeleteRequested, this, [this](const QStringList &ids) {
        int ret = QMessageBox::warning(this, QStringLiteral("确认删除"),
            QStringLiteral("确定要删除选中的 %1 个片段吗？\n此操作不可恢复。").arg(ids.size()),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;

        QStringList actuallyDeletedIds;
        for (const QString &id : ids) {
            int tabIdx = findTabForSnippet(id);
            if (tabIdx >= 0) {
                tabWidget->setCurrentIndex(tabIdx);
                currentSnippetId = id;
                if (!maybeCloseTab(tabIdx)) {
                    currentSnippetId.clear();
                    continue;
                }
                if (id == currentSnippetId)
                    currentSnippetId.clear();
                QWidget *w = tabWidget->widget(tabIdx);
                tabWidget->removeTab(tabIdx);
                w->deleteLater();
            }
            actuallyDeletedIds.append(id);
        }

        int count = snippetMgr->batchDeleteSnippets(actuallyDeletedIds);
        statusBar()->showMessage(QStringLiteral("已删除 %1 个片段").arg(count), kStatusBarShortMs);
        // Deleting snippets can remove the last owner of a tag; prune the tag
        // strip and drop any now-unused selected tag (refreshTagFilter re-runs
        // the search when it prunes).
        searchPanel->refreshTagFilter();
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

    auto startCompile = [this]() {
        m_compiling = true;
        compileAct->setEnabled(false);
        applyParamsAct->setEnabled(false);
        forceStopAct->setEnabled(true);
    };
    auto endCompile = [this]() {
        m_compiling = false;
        compileAct->setEnabled(true);
        applyParamsAct->setEnabled(true);
        forceStopAct->setEnabled(false);
    };

    connect(compileAct, &QAction::triggered, this, [this, startCompile, endCompile]() {
        if (m_compiling) {
            statusBar()->showMessage(QStringLiteral("编译正在进行中，请稍候..."), kStatusBarShortMs);
            return;
        }
        startCompile();

        saveCurrentSnippet();
        // Compiling also persists the current metadata (incl. tags); keep the
        // tag strip in sync so newly added/removed tags are reflected and any
        // now-unused selected tag is deselected.
        searchPanel->refreshTagFilter();
        QString code;
        QString templateId;
        QString snippetId = currentSnippetId;
        QString packages;
        QString tikzLibraries;
        QString compileCommand;

        if (currentSnippetId.isEmpty()) {
            CodeEditor *ed = currentEditor();
            if (!ed) { endCompile(); return; }
            code = applyParams(ed->toPlainText());
            templateId.clear();
            snippetId = "scratch";
        } else {
            Snippet s = snippetMgr->loadSnippet(currentSnippetId);
            code = applyParams(s.code);
            templateId = s.templateId;
            packages = s.packages;
            tikzLibraries = s.tikzLibraries;
            compileCommand = s.compileCommand;
        }

        if (code.trimmed().isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请先输入 TikZ 代码"), kStatusBarShortMs);
            endCompile();
            return;
        }

        logPanel->clear();
        compiler->compile(code, templateId, snippetId, packages, tikzLibraries, compileCommand);
    });

    connect(applyParamsAct, &QAction::triggered, this, [this, startCompile, endCompile]() {
        if (m_compiling) {
            statusBar()->showMessage(QStringLiteral("编译正在进行中，请稍候..."), kStatusBarShortMs);
            return;
        }
        if (currentSnippetId.isEmpty()) return;
        startCompile();

        Snippet s = snippetMgr->loadSnippet(currentSnippetId);
        QString code = applyParams(s.code);
        logPanel->clear();
        compiler->compile(code, s.templateId, currentSnippetId, s.packages, s.tikzLibraries, s.compileCommand);
    });

    connect(saveAct, &QAction::triggered, this, [this]() {
        saveCurrentSnippet();
        // Adding/removing tags on the current snippet must refresh the tag strip
        // (new tags appear; tags no longer used by any snippet disappear and are
        // deselected). refreshTagFilter() re-runs the search itself when it
        // prunes a selected tag; it leaves the category tree untouched so the
        // current category filter is preserved. A trailing refreshSearch()
        // covers the no-prune case.
        searchPanel->refreshTagFilter();
        refreshSearch();
        QSettings settings("HiTikZ", "TikzManager");
        if (settings.value("behavior/autoCompileOnSave", true).toBool()) {
            compileAct->trigger();
        }
    });

    connect(forceStopAct, &QAction::triggered, this, [this]() {
        if (m_batchGenerating) {
            m_batchCancelFlag.storeRelaxed(1);
            statusBar()->showMessage(QStringLiteral("正在终止批量生成..."), kStatusBarShortMs);
        } else if (m_compiling) {
            compiler->cancelCompile();
            statusBar()->showMessage(QStringLiteral("编译已强制中断"), kStatusBarLongMs);
        }
    });

    connect(duplicateAct, &QAction::triggered, this, [this]() {
        if (currentSnippetId.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请先选择或打开一个片段"), kStatusBarShortMs);
            return;
        }
        saveCurrentSnippet();
        Snippet orig = snippetMgr->loadSnippet(currentSnippetId);
        if (orig.id.isEmpty()) return;

        QString baseName = orig.name;
        QString newId = snippetMgr->createSnippet(baseName, orig.category);
        Snippet dup = snippetMgr->loadSnippet(newId);
        dup.code = orig.code;
        dup.description = orig.description;
        dup.tags = orig.tags;
        dup.templateId = orig.templateId;
        dup.packages = orig.packages;
        dup.tikzLibraries = orig.tikzLibraries;
        dup.compileCommand = orig.compileCommand;
        snippetMgr->saveSnippet(dup);

        refreshSearch();
        refreshCategoryTree();
        loadSnippetIntoEditor(newId);
        statusBar()->showMessage(QStringLiteral("片段已复制"), kStatusBarShortMs);
    });

    connect(compiler, &LatexCompiler::compilationFinished,
        this, [this, endCompile](bool success, const QString &pdfPath, const QString &log) {
            if (m_batchGenerating) return;
            endCompile();
            m_userCodeStartLine = compiler->userCodeStartLine();
            setFormattedLog(success, compiler->lastFullCommand(), log, m_userCodeStartLine);
            if (success) {
                pdfPreview->reloadDocument(QFileInfo(pdfPath).absoluteFilePath());
                QTimer::singleShot(kZoomApplyDelayMs, pdfPreview, &PdfPreviewWidget::applyZoomPreference);
                savePreviewData(pdfPath, currentSnippetId);
                if (m_compileStatusLabel) {
                    m_compileStatusLabel->setText(QStringLiteral("编译成功"));
                    m_compileStatusLabel->setStyleSheet("color: green; font-weight: bold; padding: 0 8px;");
                    m_compileStatusLabel->setVisible(true);
                    m_compileStatusTimer->start(kStatusBarShortMs);
                }
            } else {
                pdfPreview->clearDocument();
                jumpToErrorLine(log);
                if (m_compileStatusLabel) {
                    m_compileStatusLabel->setText(QStringLiteral("编译失败，详见日志"));
                    m_compileStatusLabel->setStyleSheet("color: red; font-weight: bold; padding: 0 8px;");
                    m_compileStatusLabel->setVisible(true);
                    m_compileStatusTimer->start(kStatusBarShortMs);
                }
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

    // Pass snippet libraries to the editor's document state for completion/highlighting
    {
        CodeEditor *editor = currentEditor();
        if (editor && editor->documentState()) {
            QStringList libs;
            if (!s.tikzLibraries.isEmpty()) {
                for (const QString &lib : s.tikzLibraries.split(',', Qt::SkipEmptyParts))
                    libs << lib.trimmed();
            }
            editor->documentState()->setSnippetLibraries(libs);
            QStringList pkgs;
            if (!s.packages.isEmpty()) {
                for (const QString &pkg : s.packages.split(',', Qt::SkipEmptyParts))
                    pkgs << pkg.trimmed();
            }
            editor->documentState()->setSnippetPackages(pkgs);
            editor->documentState()->setTemplateContent(
                templateContentFor(s.templateId));
            editor->reparseDocumentState();
        }
    }

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

    m_lastSavedCode = s.code;
    m_loadingDepth--;
    performParseParams();
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

    m_lastSavedCode = ed->toPlainText();

    int tabIdx = findTabForSnippet(currentSnippetId);
    if (tabIdx >= 0)
        tabWidget->setTabText(tabIdx, s.name.isEmpty() ? currentSnippetId.left(8) : s.name);

    clearDraft();
}

void MainWindow::saveCurrentTabUiState()
{
    // Persist under the snippet whose metadata the shared widgets currently
    // display, NOT tabWidget->currentIndex() — during a tab switch the current
    // index already points at the arriving tab, which would misattribute the
    // departing tab's edits to the wrong snippet.
    QString sid = m_uiStateSnippetId;
    if (sid.isEmpty()) return;

    TabUiState st;
    st.name          = nameEdit->text();
    st.desc          = descEdit->toPlainText();
    st.tags          = tagsEdit->text();
    st.packages      = packagesEdit->text();
    st.tikzLibraries = tikzLibrariesEdit->text();
    st.templateId    = templateCombo->currentData().toString();
    m_tabUiStates[sid] = st;
}

void MainWindow::restoreTabUiState(const QString &sid)
{
    if (!m_tabUiStates.contains(sid)) return;
    const TabUiState &st = m_tabUiStates[sid];
    nameEdit->setText(st.name);
    descEdit->setPlainText(st.desc);
    tagsEdit->setText(st.tags);
    packagesEdit->setText(st.packages);
    tikzLibrariesEdit->setText(st.tikzLibraries);
    if (!st.templateId.isEmpty()) {
        int ti = templateCombo->findData(st.templateId);
        if (ti >= 0) templateCombo->setCurrentIndex(ti);
    }
    m_tabUiStates.remove(sid); // consumed; the active UI now holds this state
}

void MainWindow::updateTabUiState()
{
    // Ignore programmatic widget updates during snippet/tab loading; only real
    // user edits should mutate the cached per-tab state and tab title.
    if (m_loadingDepth != 0) return;
    saveCurrentTabUiState();
    syncDocStateLibraries();
    onCurrentSnippetChanged();
}

void MainWindow::syncDocStateLibraries()
{
    CodeEditor *editor = currentEditor();
    if (!editor || !editor->documentState()) return;

    QStringList libs;
    if (!tikzLibrariesEdit->text().isEmpty()) {
        for (const QString &lib : tikzLibrariesEdit->text().split(',', Qt::SkipEmptyParts))
            libs << lib.trimmed();
    }
    editor->documentState()->setSnippetLibraries(libs);

    QStringList pkgs;
    if (!packagesEdit->text().isEmpty()) {
        for (const QString &pkg : packagesEdit->text().split(',', Qt::SkipEmptyParts))
            pkgs << pkg.trimmed();
    }
    editor->documentState()->setSnippetPackages(pkgs);

    // The LaTeX template also contributes packages/libraries (e.g.
    // default_circuit loads circuitikz); without this, completions gated on
    // those libraries never activate for template-provided packages.
    editor->documentState()->setTemplateContent(
        templateContentFor(templateCombo->currentData().toString()));

    editor->reparseDocumentState();
}

QString MainWindow::templateContentFor(const QString &templateId)
{
    if (templateId.isEmpty()
        || templateId.contains('/') || templateId.contains('\\')
        || templateId.contains(".."))
        return QString();
    QFile f(SettingsDialog::templateDir() + templateId + ".tex");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromUtf8(f.readAll());
}

void MainWindow::onCurrentSnippetChanged()
{
    CodeEditor *ed = currentEditor();
    if (ed) {
        int tabIdx = tabWidget->currentIndex();
        if (tabIdx >= 0) {
            QString title = nameEdit->text().isEmpty()
                ? QStringLiteral("未命名")
                : nameEdit->text();

            bool isDirty = (ed->toPlainText() != m_lastSavedCode);

            // Also detect metadata-only edits so the '*' suffix appears.
            if (!isDirty && !currentSnippetId.isEmpty()) {
                Snippet s = snippetMgr->loadSnippet(currentSnippetId);
                if (!s.id.isEmpty()) {
                    if (nameEdit->text() != s.name) isDirty = true;
                    else if (descEdit->toPlainText() != s.description) isDirty = true;
                    else if (packagesEdit->text() != s.packages) isDirty = true;
                    else if (tikzLibrariesEdit->text() != s.tikzLibraries) isDirty = true;
                    else if (tagsEdit->text() != s.tags.join(QStringLiteral(", "))) isDirty = true;
                    else if (!s.templateId.isEmpty()
                             && templateCombo->currentData().toString() != s.templateId)
                        isDirty = true;
                }
            }

            if (isDirty) {
                if (!title.endsWith(QStringLiteral(" *")))
                    title += QStringLiteral(" *");
            } else {
                if (title.endsWith(QStringLiteral(" *")))
                    title.chop(2);
            }
            tabWidget->setTabText(tabIdx, title);
        }
    }
    parseParams();
}

QString MainWindow::snippetDataPath(const QString &id) const
{
    if (id.isEmpty())
        return QString();

    if (snippetMgr->isPresetId(id))
        return snippetMgr->getPresetPath() + id;
    else
        return snippetMgr->getBasePath() + id;
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

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == logPanel->viewport() && event->type() == QEvent::MouseButtonDblClick) {
        handleLogDoubleClick();
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
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

    QString resourceDir = resolveResourceDir();
    SettingsDialog::ensureTemplatesCopied(resourceDir + "/templates");
    SnippetManager::copyPresetsFromResources(
        resourceDir + "/presets",
        dataLocation + "/presets/");
    SettingsDialog::applyToCompiler(compiler);

    snippetMgr->invalidateCaches();

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
    int uiFontSize = settings.value("ui/fontSize", kDefaultFontSize).toInt();

    QApplication::setFont(QFont(QApplication::font().family(), uiFontSize));

    QFont uiFont = QApplication::font();
    searchPanel->applyUIFont(uiFont);

    QFont editorFont("monospace", fontSize);
    QFont logFont("monospace", qMax(8, fontSize - 1));
    logPanel->setFont(logFont);

    const bool wrapLongLines = settings.value("behavior/wrapLongLines", true).toBool();
    const bool bracketHighlight = settings.value("behavior/bracketHighlight", false).toBool();

    for (int i = 0; i < tabWidget->count(); ++i) {
        CodeEditor *ed = qobject_cast<CodeEditor*>(tabWidget->widget(i));
        if (ed) {
            ed->setFont(editorFont);
            ed->setTabStopDistance(4 * ed->fontMetrics().horizontalAdvance(' '));
            ed->setWordWrap(wrapLongLines);
            ed->setBracketHighlightEnabled(bracketHighlight);
        }
    }
}
