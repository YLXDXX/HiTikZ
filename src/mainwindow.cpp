#include "mainwindow.h"
#include "search_panel.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "code_editor.h"
#include "settings_dialog.h"
#include "kde_global_shortcut.h"
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

#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define STRINGIFY_IMPL(x) #x
#define RES_DIR STRINGIFY(RESOURCE_DIR)

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentSnippetId("")
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
    connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);
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
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon && trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
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
    QAction *exportMenuAct = importExportMenu->addAction(QStringLiteral("导出当前"));
    QAction *exportAllMenuAct = importExportMenu->addAction(QStringLiteral("导出全部"));
    importExportBtn->setMenu(importExportMenu);
    toolBar->addWidget(importExportBtn);

    toolBar->addSeparator();

    compileAct = toolBar->addAction(QStringLiteral("编译预览"));
    applyParamsAct = toolBar->addAction(QStringLiteral("应用参数"));
    saveAct = toolBar->addAction(QStringLiteral("保存"));

    toolBar->addSeparator();

    QAction *copyCodeAct = toolBar->addAction(QStringLiteral("复制代码"));
    QAction *copyFullAct = toolBar->addAction(QStringLiteral("复制完整文档"));
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
    zoomOutAct = toolBar->addAction(QStringLiteral("缩小"));
    zoomInAct = toolBar->addAction(QStringLiteral("放大"));

    toolBar->addSeparator();

    QAction *settingsAct = toolBar->addAction(QStringLiteral("设置"));

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);

    searchPanel = new SearchPanel(snippetMgr);

    // --- Center Panel ---
    QSplitter *centerSplitter = new QSplitter(Qt::Vertical);
    codeEditor = new CodeEditor;
    codeEditor->setFont(QFont("monospace", 10));
    codeEditor->setTabStopDistance(4 * codeEditor->fontMetrics().horizontalAdvance(' '));
    codeEditor->setLineWrapMode(QPlainTextEdit::NoWrap);

    logPanel = new QPlainTextEdit;
    logPanel->setReadOnly(true);
    logPanel->setFont(QFont("monospace", 9));
    logPanel->setMaximumBlockCount(2000);
    logPanel->viewport()->installEventFilter(this);

    centerSplitter->addWidget(codeEditor);
    centerSplitter->addWidget(logPanel);
    centerSplitter->setSizes({600, 100});

    // --- Right Panel ---
    rightPanel = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    pdfDoc = new QPdfDocument(this);
    pdfView = new QPdfView;
    pdfView->setDocument(pdfDoc);
    pdfView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pdfView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pdfView->viewport()->installEventFilter(this);
    pdfView->viewport()->setCursor(Qt::OpenHandCursor);
    pdfView->setMinimumHeight(0);

    QWidget *metadataWidget = new QWidget;
    QVBoxLayout *metaLayout = new QVBoxLayout(metadataWidget);
    metaLayout->setContentsMargins(4, 4, 4, 4);

    QScrollArea *metaScroll = new QScrollArea;
    metaScroll->setWidgetResizable(true);
    metaScroll->setWidget(metadataWidget);
    metaScroll->setMinimumHeight(0);

    QSplitter *rightSplitter = new QSplitter(Qt::Vertical);
    rightSplitter->addWidget(pdfView);
    rightSplitter->addWidget(metaScroll);
    rightSplitter->setStretchFactor(0, 3);
    rightSplitter->setStretchFactor(1, 1);
    rightSplitter->setSizes({400, 300});
    rightSplitter->setChildrenCollapsible(false);
    rightLayout->addWidget(rightSplitter);

    connect(fitPageAct, &QAction::triggered, this, &MainWindow::fitPdfPage);
    connect(fitWidthAct, &QAction::triggered, this, &MainWindow::fitPdfWidth);
    connect(fitHeightAct, &QAction::triggered, this, &MainWindow::fitPdfHeight);
    connect(zoomInAct, &QAction::triggered, this, &MainWindow::zoomPdfIn);
    connect(zoomOutAct, &QAction::triggered, this, &MainWindow::zoomPdfOut);

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
    // --- Toolbar actions ---
    connect(newAct, &QAction::triggered, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle(QStringLiteral("新建片段"));
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
            refreshSearch();
            refreshCategoryTree();
            loadSnippetIntoEditor(id);
        }
    });

    connect(deleteAct, &QAction::triggered, this, [this]() {
        QString cat = searchPanel->currentCategory();
        bool isAllCat = cat.isEmpty();

        if (currentSnippetId.isEmpty() && !cat.isEmpty() && !isAllCat) {
            int ret = QMessageBox::warning(this, QStringLiteral("删除分类"),
                QStringLiteral("确定删除分类 \"%1\" 及其全部内容吗？").arg(cat),
                QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                int count = snippetMgr->deleteCategory(cat);
                statusBar()->showMessage(QStringLiteral("已删除 %1 个片段").arg(count), 3000);
                codeEditor->clear();
                nameEdit->clear();
                tagsEdit->clear();
                packagesEdit->clear();
                tikzLibrariesEdit->clear();
                descEdit->clear();
                pdfDoc->close();
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
            currentSnippetId.clear();
            codeEditor->clear();
            nameEdit->clear();
            tagsEdit->clear();
            packagesEdit->clear();
            tikzLibrariesEdit->clear();
            descEdit->clear();
            pdfDoc->close();
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
                statusBar()->showMessage(QStringLiteral("成功导入 %1 个片段").arg(imported.size()), 5000);
                refreshCategoryTree();
                refreshSearch();
                if (!imported.isEmpty()) {
                    loadSnippetIntoEditor(imported.first());
                }
            }
        }
    });

    connect(exportMenuAct, &QAction::triggered, this, [this]() {
        if (currentSnippetId.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请先选择一个片段"), 3000);
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出存档"), "", "TikZ 存档 (*.tar.gz)");
        if (!filePath.isEmpty()) {
            if (snippetMgr->exportSnippetZip(currentSnippetId, filePath)) {
                statusBar()->showMessage(QStringLiteral("导出成功"), 3000);
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
            statusBar()->showMessage(QStringLiteral("没有可导出的片段"), 3000);
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出全部存档"), "all_snippets.tar.gz", "TikZ 存档 (*.tar.gz)");
        if (!filePath.isEmpty()) {
            QStringList allIds;
            for (const Snippet &s : all)
                allIds.append(s.id);
            if (snippetMgr->exportSnippetsZip(allIds, filePath))
                statusBar()->showMessage(QStringLiteral("全部导出成功"), 3000);
            else
                QMessageBox::warning(this, QStringLiteral("导出失败"),
                    QStringLiteral("无法导出全部片段。"));
        }
    });

    auto copyCode = [this]() {
        QString code = applyParams(codeEditor->toPlainText());
        static QRegularExpression paramLine("^%\\s*@param:.*(\n|\r\n?)?", QRegularExpression::MultilineOption);
        code.remove(paramLine);
        QApplication::clipboard()->setText(code);
        statusBar()->showMessage(QStringLiteral("代码已复制到剪贴板"), 2000);
    };

    auto copyFullDocument = [this]() {
        if (currentSnippetId.isEmpty()) {
            QString code = applyParams(codeEditor->toPlainText());
            static QRegularExpression paramLine("^%\\s*@param:.*(\n|\r\n?)?", QRegularExpression::MultilineOption);
            code.remove(paramLine);
            QString fullDoc = compiler->wrapCode(code, QString(), QString(), QString());
            QApplication::clipboard()->setText(fullDoc);
            statusBar()->showMessage(QStringLiteral("完整文档已复制到剪贴板"), 2000);
            return;
        }
        Snippet s = snippetMgr->loadSnippet(currentSnippetId);
        QString code = applyParams(s.code);
        static QRegularExpression paramLine("^%\\s*@param:.*(\n|\r\n?)?", QRegularExpression::MultilineOption);
        code.remove(paramLine);
        QString fullDoc = compiler->wrapCode(code, s.templateId, s.packages, s.tikzLibraries);
        QApplication::clipboard()->setText(fullDoc);
        statusBar()->showMessage(QStringLiteral("完整文档已复制到剪贴板"), 2000);
    };

    connect(copyCodeAct, &QAction::triggered, this, copyCode);
    connect(copyFullAct, &QAction::triggered, this, copyFullDocument);

    auto copyPngFromCurrentPreview = [this]() {
        QString pdfPath;
        if (!currentSnippetId.isEmpty()) {
            pdfPath = snippetDataPath(currentSnippetId) + "/preview.pdf";
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            pdfPath = compiler->pdfPath();
        }
        if (!pdfPath.isEmpty() && QFile::exists(pdfPath)) {
            compiler->convertToPng(pdfPath, 300);
            connect(compiler, &LatexCompiler::conversionFinished, this,
                [this](bool ok, const QString &pngPath) {
                    if (ok) {
                        QImage img(pngPath);
                        if (!img.isNull()) {
                            QApplication::clipboard()->setImage(img);
                            statusBar()->showMessage(QStringLiteral("PNG已复制到剪贴板"), 2000);
                        }
                    }
                }, Qt::SingleShotConnection);
        } else {
            statusBar()->showMessage(QStringLiteral("请先编译生成PDF预览"), 3000);
        }
    };

    auto copySvgFromCurrentPreview = [this]() {
        QString pdfPath;
        if (!currentSnippetId.isEmpty()) {
            pdfPath = snippetDataPath(currentSnippetId) + "/preview.pdf";
        }
        if (pdfPath.isEmpty() || !QFile::exists(pdfPath)) {
            pdfPath = compiler->pdfPath();
        }
        if (!pdfPath.isEmpty() && QFile::exists(pdfPath)) {
            compiler->convertToSvg(pdfPath);
            connect(compiler, &LatexCompiler::conversionFinished, this,
                [this](bool ok, const QString &svgPath) {
                    if (ok) {
                        QFile svgFile(svgPath);
                        if (svgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            QByteArray svgData = svgFile.readAll();
                            svgFile.close();
                            QMimeData *mimeData = new QMimeData;
                            mimeData->setData(QStringLiteral("image/svg+xml"), svgData);
                            mimeData->setText(QString::fromUtf8(svgData));
                            QApplication::clipboard()->setMimeData(mimeData);
                            statusBar()->showMessage(QStringLiteral("SVG已复制到剪贴板"), 2000);
                        }
                    }
                }, Qt::SingleShotConnection);
        } else {
            statusBar()->showMessage(QStringLiteral("请先编译生成PDF预览"), 3000);
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
            statusBar()->showMessage(QStringLiteral("设置已保存"), 3000);
        }
    });

    copyCodeShortcut = new QShortcut(this);
    copyPngShortcut = new QShortcut(this);
    copySvgShortcut = new QShortcut(this);
    compileShortcut = new QShortcut(this);
    applyParamsShortcut = new QShortcut(this);
    saveShortcut = new QShortcut(this);
    connect(copyCodeShortcut, &QShortcut::activated, this, copyCode);
    connect(copyPngShortcut, &QShortcut::activated, this, copyPngFromCurrentPreview);
    connect(copySvgShortcut, &QShortcut::activated, this, copySvgFromCurrentPreview);
    connect(compileShortcut, &QShortcut::activated, this, [this]() { compileAct->trigger(); });
    connect(applyParamsShortcut, &QShortcut::activated, this, [this]() { applyParamsAct->trigger(); });
    connect(saveShortcut, &QShortcut::activated, this, [this]() { saveAct->trigger(); });
    applyShortcuts();

    mainSplitter->addWidget(searchPanel);
    mainSplitter->addWidget(centerSplitter);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({250, 600, 350});

    setCentralWidget(mainSplitter);
    statusBar()->showMessage(QStringLiteral("就绪"), 3000);
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
                statusBar()->showMessage(QStringLiteral("批量导出成功 (%1 个片段)").arg(ids.size()), 3000);
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
            statusBar()->showMessage(QStringLiteral("已修改 %1 个片段的分类").arg(ids.size()), 3000);
            searchPanel->refreshCategoryTree();
            searchPanel->refreshSearch();
        }
    });

    connect(searchPanel, &SearchPanel::batchDeleteRequested, this, [this](const QStringList &ids) {
        int ret = QMessageBox::warning(this, QStringLiteral("确认删除"),
            QStringLiteral("确定要删除选中的 %1 个片段吗？\n此操作不可恢复。").arg(ids.size()),
            QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;

        if (ids.contains(currentSnippetId)) {
            currentSnippetId.clear();
            codeEditor->clear();
            nameEdit->clear();
            tagsEdit->clear();
            packagesEdit->clear();
            tikzLibrariesEdit->clear();
            descEdit->clear();
            pdfDoc->close();
        }

        int count = snippetMgr->batchDeleteSnippets(ids);
        statusBar()->showMessage(QStringLiteral("已删除 %1 个片段").arg(count), 3000);
        searchPanel->refreshCategoryTree();
        searchPanel->refreshSearch();
    });

    connect(searchPanel, &SearchPanel::exportAllRequested, this, [this]() {
        QList<Snippet> all = snippetMgr->getAllSnippets(true);
        all.append(snippetMgr->getAllPresets(true));
        if (all.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("没有可导出的片段"), 3000);
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出全部存档"), "all_snippets.tar.gz", "TikZ 存档 (*.tar.gz)");
        if (!filePath.isEmpty()) {
            QStringList allIds;
            for (const Snippet &s : all)
                allIds.append(s.id);
            if (snippetMgr->exportSnippetsZip(allIds, filePath))
                statusBar()->showMessage(QStringLiteral("全部导出成功"), 3000);
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
            code = codeEditor->toPlainText();
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
            statusBar()->showMessage(QStringLiteral("请先输入 TikZ 代码"), 3000);
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
            setFormattedLog(log);
            if (success) {
                pdfDoc->close();
                pdfDoc->load(QFileInfo(pdfPath).absoluteFilePath());
                QTimer::singleShot(200, this, [this]() { applyPdfZoomPreference(); });
                if (!m_batchGenerating) {
                    savePreviewData(pdfPath, currentSnippetId);
                }
                statusBar()->showMessage(QStringLiteral("编译成功"), 3000);
            } else {
                pdfDoc->close();
                jumpToErrorLine(log);
                statusBar()->showMessage(QStringLiteral("编译失败，详见日志"), 3000);
            }
        });

    connect(codeEditor, &QPlainTextEdit::textChanged, this, [this]() {
        if (!currentSnippetId.isEmpty()) {
            onCurrentSnippetChanged();
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
}

void MainWindow::loadSnippetIntoEditor(const QString &id)
{
    Snippet s = snippetMgr->loadSnippet(id);
    if (s.id.isEmpty()) return;

    currentSnippetId = id;
    {
        const QSignalBlocker blocker(codeEditor);
        codeEditor->setPlainText(s.code);
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

    parseParams();
}

void MainWindow::saveCurrentSnippet()
{
    if (currentSnippetId.isEmpty()) return;

    Snippet s = snippetMgr->loadSnippet(currentSnippetId);
    s.name = nameEdit->text();
    s.description = descEdit->toPlainText();
    s.code = codeEditor->toPlainText();
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
}

void MainWindow::onCurrentSnippetChanged()
{
    parseParams();
}

void MainWindow::jumpToErrorLine(const QString &logText)
{
    QRegularExpression re("l\\.(\\d+)");
    QRegularExpressionMatchIterator it = re.globalMatch(logText);
    if (!it.hasNext()) return;

    QRegularExpressionMatch match = it.next();
    int line = match.captured(1).toInt();
    if (line < 1) return;

    QTextCursor cursor = codeEditor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
    codeEditor->setTextCursor(cursor);
    codeEditor->highlightCurrentLine();
    codeEditor->setFocus();
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

    QString code = codeEditor->toPlainText();
    QRegularExpression re("%\\s*@param:\\s*(\\w+)=(\\S+)");
    QRegularExpressionMatchIterator it = re.globalMatch(code);

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
    }

    paramsLayout->addStretch();
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
    pdfDoc->close();
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
    QFile::copy(pdfPath, previewPdf);

    if (m_batchGenerating) {
        QProcess pngProc;
        QStringList args;
        args << "-png" << "-r" << "150" << "-singlefile" << pdfPath << (basePath + "/preview");
        pngProc.start("pdftocairo", args);
        pngProc.waitForFinished(10000);
    } else {
        QProcess *pngProc = new QProcess(this);
        connect(pngProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            pngProc, &QProcess::deleteLater);
        QStringList args;
        args << "-png" << "-r" << "150" << "-singlefile" << pdfPath << (basePath + "/preview");
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
        pdfDoc->close();
        pdfDoc->load(QFileInfo(previewPdf).absoluteFilePath());
        QTimer::singleShot(200, this, [this]() { applyPdfZoomPreference(); });
    } else {
        clearPdfPreview();
    }
}

void MainWindow::generateAllPreviews()
{
    QList<Snippet> all = snippetMgr->getAllSnippets(true);
    all.append(snippetMgr->getAllPresets(true));

    if (all.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("没有可生成预览的条目"), 3000);
        return;
    }

    m_batchGenerating = true;
    statusBar()->showMessage(QStringLiteral("正在生成所有预览..."), 0);

    int done = 0;
    int total = all.size();

    for (const Snippet &s : all) {
        QEventLoop loop;
        QTimer::singleShot(30000, &loop, &QEventLoop::quit);

        QString snippetId = s.id;
        QString code = resolveParamsFromCode(s.code);

        QObject::connect(compiler, &LatexCompiler::compilationFinished,
            &loop, [&](bool success, const QString &pdfPath, const QString &) {
                if (success) {
                    savePreviewData(pdfPath, snippetId);
                }
                done++;
                statusBar()->showMessage(
                    QStringLiteral("生成预览: %1/%2").arg(done).arg(total), 0);
                loop.quit();
            }, Qt::SingleShotConnection);

        compiler->compile(code, s.templateId, snippetId, s.packages, s.tikzLibraries);
        loop.exec();
    }

    statusBar()->showMessage(QStringLiteral("预览生成完毕: %1 个条目").arg(total), 5000);
    m_batchGenerating = false;
    refreshSearch();
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
    templateCombo->setCurrentIndex(0);
}

void MainWindow::setFormattedLog(const QString &log)
{
    logPanel->clear();

    if (log.isEmpty()) return;

    QTextCharFormat errorFormat;
    errorFormat.setForeground(QColor(220, 30, 30));
    errorFormat.setFontWeight(QFont::Bold);

    QTextCharFormat warningFormat;
    warningFormat.setForeground(QColor(200, 140, 0));

    QTextCharFormat lineNumFormat;
    lineNumFormat.setForeground(QColor(180, 60, 60));

    QTextCharFormat infoFormat;
    infoFormat.setForeground(QColor(100, 100, 100));

    QTextCharFormat defaultFormat;

    const QStringList lines = log.split('\n');
    static const QRegularExpression errorRe("^!\\s");
    static const QRegularExpression warningRe("Warning", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression lineRe("^l\\.\\d+");
    static const QRegularExpression infoRe("^\\(");
    static const QRegularExpression overfullRe("Overfull|Underfull");

    for (const QString &line : lines) {
        QTextCharFormat fmt = defaultFormat;
        if (errorRe.match(line).hasMatch()) {
            fmt = errorFormat;
        } else if (lineRe.match(line.trimmed()).hasMatch()) {
            fmt = lineNumFormat;
        } else if (warningRe.match(line).hasMatch() || overfullRe.match(line).hasMatch()) {
            fmt = warningFormat;
        } else if (infoRe.match(line).hasMatch()) {
            fmt = infoFormat;
        }
        logPanel->textCursor().insertText(line + '\n', fmt);
    }

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

    if (obj == pdfView->viewport()) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *we = static_cast<QWheelEvent *>(event);
            if (we->angleDelta().y() > 0)
                zoomPdfIn();
            else if (we->angleDelta().y() < 0)
                zoomPdfOut();
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                m_pdfPanning = true;
                m_pdfPanStart = me->pos();
                pdfView->viewport()->setCursor(Qt::ClosedHandCursor);
                return true;
            }
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton && m_pdfPanning) {
                m_pdfPanning = false;
                pdfView->viewport()->setCursor(Qt::OpenHandCursor);
                return true;
            }
        }

        if (event->type() == QEvent::MouseMove && m_pdfPanning) {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            QPoint delta = m_pdfPanStart - me->pos();
            m_pdfPanStart = me->pos();
            if (pdfView->horizontalScrollBar())
                pdfView->horizontalScrollBar()->setValue(
                    pdfView->horizontalScrollBar()->value() + delta.x());
            if (pdfView->verticalScrollBar())
                pdfView->verticalScrollBar()->setValue(
                    pdfView->verticalScrollBar()->value() + delta.y());
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::handleLogDoubleClick()
{
    QTextCursor cursor = logPanel->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    QString line = cursor.selectedText().trimmed();

    QRegularExpression re("l\\.(\\d+)");
    QRegularExpressionMatch match = re.match(line);
    if (!match.hasMatch()) return;

    int lineNum = match.captured(1).toInt();
    if (lineNum < 1) return;

    cursor = codeEditor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNum - 1);
    codeEditor->setTextCursor(cursor);
    codeEditor->highlightCurrentLine();
    codeEditor->setFocus();
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

void MainWindow::zoomPdfIn()
{
    QPoint vpPos = pdfView->viewport() ? pdfView->viewport()->mapFromGlobal(QCursor::pos()) : QPoint();
    qreal oldFactor = pdfView->zoomFactor();
    qreal newFactor = oldFactor * 1.25;

    pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    pdfView->setZoomFactor(newFactor);

    if (pdfView->viewport()) {
        qreal scale = newFactor / oldFactor;
        QPoint delta(vpPos.x() * (scale - 1.0), vpPos.y() * (scale - 1.0));
        if (pdfView->horizontalScrollBar())
            pdfView->horizontalScrollBar()->setValue(
                pdfView->horizontalScrollBar()->value() + delta.x());
        if (pdfView->verticalScrollBar())
            pdfView->verticalScrollBar()->setValue(
                pdfView->verticalScrollBar()->value() + delta.y());
    }
}

void MainWindow::zoomPdfOut()
{
    QPoint vpPos = pdfView->viewport() ? pdfView->viewport()->mapFromGlobal(QCursor::pos()) : QPoint();
    qreal oldFactor = pdfView->zoomFactor();
    qreal newFactor = qMax(0.1, oldFactor / 1.25);

    pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    pdfView->setZoomFactor(newFactor);

    if (pdfView->viewport()) {
        qreal scale = newFactor / oldFactor;
        QPoint delta(vpPos.x() * (scale - 1.0), vpPos.y() * (scale - 1.0));
        if (pdfView->horizontalScrollBar())
            pdfView->horizontalScrollBar()->setValue(
                pdfView->horizontalScrollBar()->value() + delta.x());
        if (pdfView->verticalScrollBar())
            pdfView->verticalScrollBar()->setValue(
                pdfView->verticalScrollBar()->value() + delta.y());
    }
}

void MainWindow::fitPdfPage()
{
    m_pdfZoomPref = 0;
    fitPageAct->setChecked(true);
    fitWidthAct->setChecked(false);
    fitHeightAct->setChecked(false);
    pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
}

void MainWindow::fitPdfWidth()
{
    m_pdfZoomPref = 1;
    fitPageAct->setChecked(false);
    fitWidthAct->setChecked(true);
    fitHeightAct->setChecked(false);
    pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth);
}

void MainWindow::fitPdfHeight()
{
    if (!pdfDoc || pdfDoc->status() != QPdfDocument::Status::Ready)
        return;

    QSizeF pageSize = pdfDoc->pagePointSize(0);
    if (pageSize.isEmpty()) return;

    QSize vpSize = pdfView->viewport() ? pdfView->viewport()->size() : QSize();
    if (vpSize.height() <= 0) return;

    qreal dpi = pdfView->logicalDpiY();
    qreal pagePixels = pageSize.height() * dpi / 72.0;
    qreal ratio = vpSize.height() / pagePixels;

    m_pdfZoomPref = 2;
    fitPageAct->setChecked(false);
    fitWidthAct->setChecked(false);
    fitHeightAct->setChecked(true);
    pdfView->setZoomMode(QPdfView::ZoomMode::Custom);
    pdfView->setZoomFactor(ratio);
}

void MainWindow::applyPdfZoomPreference()
{
    if (!pdfDoc || pdfDoc->status() != QPdfDocument::Status::Ready)
        return;

    fitPageAct->setChecked(m_pdfZoomPref == 0);
    fitWidthAct->setChecked(m_pdfZoomPref == 1);
    fitHeightAct->setChecked(m_pdfZoomPref == 2);

    switch (m_pdfZoomPref) {
    case 0: pdfView->setZoomMode(QPdfView::ZoomMode::FitInView); break;
    case 1: pdfView->setZoomMode(QPdfView::ZoomMode::FitToWidth); break;
    case 2: fitPdfHeight(); break;
    }
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
    codeEditor->clear();
    nameEdit->clear();
    descEdit->clear();
    tagsEdit->clear();
    packagesEdit->clear();
    tikzLibrariesEdit->clear();
    clearPdfPreview();
    refreshCategoryTree();
    refreshSearch();
}

void MainWindow::setFitPageChecked(bool checked) { fitPageAct->setChecked(checked); }
void MainWindow::setFitWidthChecked(bool checked) { fitWidthAct->setChecked(checked); }
void MainWindow::setFitHeightChecked(bool checked) { fitHeightAct->setChecked(checked); }
