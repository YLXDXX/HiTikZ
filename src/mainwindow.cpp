#include "mainwindow.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "code_editor.h"
#include "settings_dialog.h"
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
                searchBox->setFocus();
            }
        }
    });

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

#ifdef HAS_QHOTKEY
    QHotkey *hotkey = new QHotkey(QKeySequence("Ctrl+Alt+T"), true, this);
    if (hotkey->isRegistered()) {
        connect(hotkey, &QHotkey::activated, this, [this]() {
            if (isVisible() && !isMinimized()) {
                hide();
            } else {
                show();
                raise();
                activateWindow();
                searchBox->setFocus();
            }
        });
    } else {
        qWarning() << "Global hotkey (Ctrl+Alt+T) unavailable (not supported on this platform)";
    }
#endif
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
    QAction *importAct = toolBar->addAction(QStringLiteral("导入ZIP"));
    QAction *exportAct = toolBar->addAction(QStringLiteral("导出ZIP"));
    toolBar->addSeparator();
    QAction *copyCodeAct = toolBar->addAction(QStringLiteral("复制代码 (Ctrl+Shift+C)"));
    QAction *copyPngAct = toolBar->addAction(QStringLiteral("复制PNG (Ctrl+Shift+P)"));
    QAction *copySvgAct = toolBar->addAction(QStringLiteral("复制SVG (Ctrl+Shift+S)"));
    toolBar->addSeparator();
    QAction *settingsAct = toolBar->addAction(QStringLiteral("设置"));
    toolBar->addSeparator();
    QAction *genPreviewAct = toolBar->addAction(QStringLiteral("生成所有预览"));
    QAction *resetAct = toolBar->addAction(QStringLiteral("重置出厂"));

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);

    // --- Left Panel ---
    leftPanel = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(4, 4, 4, 4);

    searchBox = new QLineEdit;
    searchBox->setPlaceholderText(QStringLiteral("搜索名称或简介..."));
    searchBox->setClearButtonEnabled(true);

    categoryTree = new QTreeView;
    categoryTree->setHeaderHidden(true);
    categoryTree->setRootIsDecorated(true);
    categoryTree->setContextMenuPolicy(Qt::CustomContextMenu);
    categoryTree->setAcceptDrops(true);
    categoryTree->setDropIndicatorShown(true);
    categoryTree->setDragDropMode(QAbstractItemView::DropOnly);
    categoryTree->viewport()->setAcceptDrops(true);
    categoryModel = new QStandardItemModel(this);
    categoryTree->setModel(categoryModel);
    categoryTree->viewport()->installEventFilter(this);

    thumbnailList = new QListView;
    thumbnailList->setViewMode(QListView::IconMode);
    thumbnailList->setIconSize(QSize(96, 96));
    thumbnailList->setGridSize(QSize(120, 130));
    thumbnailList->setResizeMode(QListView::Adjust);
    thumbnailList->setWordWrap(true);
    thumbnailList->setDragEnabled(true);
    thumbnailList->setDragDropMode(QAbstractItemView::DragOnly);
    thumbnailModel = new QStandardItemModel(this);
    thumbnailList->setModel(thumbnailModel);

    categoryCtxMenu = new QMenu(this);
    QAction *renameCatAct = categoryCtxMenu->addAction(QStringLiteral("重命名分类"));
    QAction *deleteCatAct = categoryCtxMenu->addAction(QStringLiteral("删除分类"));
    QAction *newSubCatAct = categoryCtxMenu->addAction(QStringLiteral("新建子分类"));
    QAction *newTopCatAct = categoryCtxMenu->addAction(QStringLiteral("新建大类"));
    newTopCatAct->setVisible(false);

    auto getEffectiveCatItem = [this]() -> QStandardItem* {
        QModelIndex idx = categoryTree->currentIndex();
        return idx.isValid() ? categoryModel->itemFromIndex(idx) : nullptr;
    };

    connect(renameCatAct, &QAction::triggered, this, [this, getEffectiveCatItem]() {
        renameCategoryItem(getEffectiveCatItem());
    });
    connect(deleteCatAct, &QAction::triggered, this, [this, getEffectiveCatItem]() {
        deleteCategoryItem(getEffectiveCatItem());
    });
    connect(newSubCatAct, &QAction::triggered, this, [this, getEffectiveCatItem]() {
        QStandardItem *item = getEffectiveCatItem();
        if (!item) return;
        QString parentCat = item->data(Qt::UserRole).toString();
        bool ok;
        QString name = QInputDialog::getText(this, QStringLiteral("新建子分类"),
            QStringLiteral("子分类名称:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            QString newCat = parentCat.isEmpty() ? name : parentCat + "/" + name;
            snippetMgr->createSnippet(QStringLiteral("新片段"), newCat);
            refreshCategoryTree();
            refreshSearch();
        }
    });
    connect(newTopCatAct, &QAction::triggered, this, [this]() {
        bool ok;
        QString name = QInputDialog::getText(this, QStringLiteral("新建大类"),
            QStringLiteral("大类名称:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            snippetMgr->createSnippet(QStringLiteral("新片段"), name);
            refreshCategoryTree();
            refreshSearch();
        }
    });

    leftLayout->addWidget(searchBox);
    leftLayout->addWidget(categoryTree, 1);
    leftLayout->addWidget(thumbnailList, 2);

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

    centerSplitter->addWidget(codeEditor);
    centerSplitter->addWidget(logPanel);
    centerSplitter->setSizes({600, 100});

    // --- Right Panel ---
    rightPanel = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(4, 4, 4, 4);

    pdfDoc = new QPdfDocument(this);
    pdfView = new QPdfView;
    pdfView->setDocument(pdfDoc);
    pdfView->setMinimumSize(250, 250);

    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText(QStringLiteral("名称"));
    descEdit = new QTextEdit;
    descEdit->setPlaceholderText(QStringLiteral("简介"));
    descEdit->setMaximumHeight(80);

    compileBtn = new QPushButton(QStringLiteral("编译预览"));
    saveBtn = new QPushButton(QStringLiteral("保存"));

    rightLayout->addWidget(pdfView, 3);
    rightLayout->addWidget(new QLabel(QStringLiteral("名称:")));
    rightLayout->addWidget(nameEdit);
    rightLayout->addWidget(new QLabel(QStringLiteral("简介:")));
    rightLayout->addWidget(descEdit, 1);
    rightLayout->addWidget(new QLabel(QStringLiteral("模板:")));
    templateCombo = new QComboBox;
    rightLayout->addWidget(templateCombo);

    paramsScrollArea = new QScrollArea;
    paramsScrollArea->setWidgetResizable(true);
    paramsScrollArea->setMaximumHeight(150);
    paramsWidget = new QWidget;
    paramsLayout = new QVBoxLayout(paramsWidget);
    paramsLayout->setContentsMargins(0, 0, 0, 0);
    paramsScrollArea->setWidget(paramsWidget);
    rightLayout->addWidget(new QLabel(QStringLiteral("参数:")));
    rightLayout->addWidget(paramsScrollArea);

    applyParamsBtn = new QPushButton(QStringLiteral("应用参数"));

    rightLayout->addWidget(compileBtn);
    rightLayout->addWidget(applyParamsBtn);
    rightLayout->addWidget(saveBtn);
    // --- Toolbar actions ---
    connect(newAct, &QAction::triggered, this, [this]() {
        bool ok;
        QString name = QInputDialog::getText(this, QStringLiteral("新建片段"),
            QStringLiteral("片段名称:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            QString id = snippetMgr->createSnippet(name, "");
            refreshSearch();
            refreshCategoryTree();
            loadSnippetIntoEditor(id);
        }
    });

    connect(deleteAct, &QAction::triggered, this, [this]() {
        QModelIndex catIdx = categoryTree->currentIndex();
        QString cat = catIdx.isValid() ? catIdx.data(Qt::UserRole).toString() : QString();
        bool isAllCat = cat.isEmpty() && catIdx.isValid()
            && categoryModel->itemFromIndex(catIdx)
            && categoryModel->itemFromIndex(catIdx)->text() == QStringLiteral("全部");

        if (currentSnippetId.isEmpty() && !cat.isEmpty() && !isAllCat) {
            int ret = QMessageBox::warning(this, QStringLiteral("删除分类"),
                QStringLiteral("确定删除分类 \"%1\" 及其全部内容吗？").arg(cat),
                QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                int count = snippetMgr->deleteCategory(cat);
                statusBar()->showMessage(QStringLiteral("已删除 %1 个片段").arg(count), 3000);
                codeEditor->clear();
                nameEdit->clear();
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
            descEdit->clear();
            pdfDoc->close();
            refreshSearch();
            refreshCategoryTree();
        }
    });

    connect(importAct, &QAction::triggered, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this,
            QStringLiteral("导入ZIP"), "", "ZIP files (*.zip)");
        if (!filePath.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("ZIP导入功能将在后续版本实现"), 3000);
        }
    });

    connect(exportAct, &QAction::triggered, this, [this]() {
        if (currentSnippetId.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请先选择一个片段"), 3000);
            return;
        }
        QString filePath = QFileDialog::getSaveFileName(this,
            QStringLiteral("导出ZIP"), "", "ZIP files (*.zip)");
        if (!filePath.isEmpty()) {
            statusBar()->showMessage(QStringLiteral("ZIP导出功能将在后续版本实现"), 3000);
        }
    });

    auto copyCode = [this]() {
        QString code = applyParams(codeEditor->toPlainText());
        static QRegularExpression paramLine("^%\\s*@param:.*(\n|\r\n?)?", QRegularExpression::MultilineOption);
        code.remove(paramLine);
        QApplication::clipboard()->setText(code);
        statusBar()->showMessage(QStringLiteral("代码已复制到剪贴板"), 2000);
    };

    connect(copyCodeAct, &QAction::triggered, this, copyCode);

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
                            QString svgContent = QString::fromUtf8(svgFile.readAll());
                            svgFile.close();
                            QApplication::clipboard()->setText(svgContent);
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
            statusBar()->showMessage(QStringLiteral("设置已保存"), 3000);
        }
    });

    connect(genPreviewAct, &QAction::triggered, this, &MainWindow::generateAllPreviews);

    connect(resetAct, &QAction::triggered, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this, QStringLiteral("确认重置"),
            QStringLiteral("这将删除所有用户创建的片段和修改，恢复为初始状态。\n\n建议重启程序以生效。\n\n确定要继续吗？"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;

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
        clearPdfPreview();
        refreshCategoryTree();
        refreshSearch();

        QMessageBox::information(this, QStringLiteral("重置完成"),
            QStringLiteral("已恢复到出厂设置。"));
    });

    QShortcut *copyCodeShortcut = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    connect(copyCodeShortcut, &QShortcut::activated, this, copyCode);

    QShortcut *copyPngShortcut = new QShortcut(QKeySequence("Ctrl+Shift+P"), this);
    connect(copyPngShortcut, &QShortcut::activated, this, copyPngFromCurrentPreview);

    QShortcut *copySvgShortcut = new QShortcut(QKeySequence("Ctrl+Shift+S"), this);
    connect(copySvgShortcut, &QShortcut::activated, this, copySvgFromCurrentPreview);

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(centerSplitter);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({250, 600, 350});

    setCentralWidget(mainSplitter);
    statusBar()->showMessage(QStringLiteral("就绪"), 3000);
}

void MainWindow::setupConnections()
{
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::refreshSearch);

    connect(thumbnailList->selectionModel(), &QItemSelectionModel::currentChanged,
        this, [this](const QModelIndex &current, const QModelIndex &) {
            if (!current.isValid()) return;
            QString id = current.data(Qt::UserRole).toString();
            if (!id.isEmpty()) {
                loadSnippetIntoEditor(id);
            }
        });

    connect(categoryTree->selectionModel(), &QItemSelectionModel::currentChanged,
        this, [this](const QModelIndex &current, const QModelIndex &) {
            if (!current.isValid()) return;
            QString category = current.data(Qt::UserRole).toString();
            thumbnailModel->clear();

            currentSnippetId.clear();
            codeEditor->clear();
            nameEdit->clear();
            descEdit->clear();
            clearPdfPreview();
            clearParams();

            QList<SearchResult> results = snippetMgr->searchSnippets("");
            for (const SearchResult &r : results) {
                if (!category.isEmpty() && !r.snippet.category.startsWith(category))
                    continue;
                QString label = r.snippet.isPreset ? QStringLiteral("[预设] ") + r.snippet.name : r.snippet.name;
                QStandardItem *item = new QStandardItem(label);
                item->setData(r.snippet.id, Qt::UserRole);
                item->setToolTip(r.snippet.description);

                QIcon icon = loadThumbnailIcon(r.snippet.id, r.snippet.isPreset);
                if (!icon.isNull())
                    item->setIcon(icon);

                thumbnailModel->appendRow(item);
            }
        });

    connect(categoryTree, &QTreeView::customContextMenuRequested,
        this, &MainWindow::showCategoryContextMenu);

    connect(snippetMgr, &SnippetManager::categoriesChanged,
        this, &MainWindow::refreshCategoryTree);

    connect(templateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) {
            if (currentSnippetId.isEmpty()) return;
            Snippet s = snippetMgr->loadSnippet(currentSnippetId);
            s.templateId = templateCombo->currentData().toString();
            snippetMgr->saveSnippet(s);
        });

    connect(compileBtn, &QPushButton::clicked, this, [this]() {
        saveCurrentSnippet();
        QString code;
        QString templateId;
        QString snippetId = currentSnippetId;

        if (currentSnippetId.isEmpty()) {
            code = codeEditor->toPlainText();
            templateId.clear();
            snippetId = "scratch";
        } else {
            Snippet s = snippetMgr->loadSnippet(currentSnippetId);
            code = applyParams(s.code);
            templateId = s.templateId;
        }

        if (code.trimmed().isEmpty()) {
            statusBar()->showMessage(QStringLiteral("请先输入 TikZ 代码"), 3000);
            return;
        }

        logPanel->clear();
        compiler->compile(code, templateId, snippetId);
    });

    connect(applyParamsBtn, &QPushButton::clicked, this, [this]() {
        if (currentSnippetId.isEmpty()) return;
        Snippet s = snippetMgr->loadSnippet(currentSnippetId);
        QString code = applyParams(s.code);
        logPanel->clear();
        compiler->compile(code, s.templateId, currentSnippetId);
    });

    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        saveCurrentSnippet();
        refreshSearch();
    });

    connect(compiler, &LatexCompiler::compilationFinished,
        this, [this](bool success, const QString &pdfPath, const QString &log) {
            logPanel->setPlainText(log);
            if (success) {
                pdfDoc->close();
                pdfDoc->load(QFileInfo(pdfPath).absoluteFilePath());
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
    QString query = searchBox->text().trimmed();
    thumbnailModel->clear();

    QList<SearchResult> results = snippetMgr->searchSnippets(query);
    for (const SearchResult &r : results) {
        QString label = r.snippet.isPreset ? QStringLiteral("[预设] ") + r.snippet.name : r.snippet.name;
        QStandardItem *item = new QStandardItem(label);
        item->setData(r.snippet.id, Qt::UserRole);
        item->setToolTip(QString("%1%2 (分数: %3)\n%4")
            .arg(r.snippet.isPreset ? QStringLiteral("[预设] ") : QString())
            .arg(r.snippet.name).arg(r.score).arg(r.snippet.description));

        QIcon icon = loadThumbnailIcon(r.snippet.id, r.snippet.isPreset);
        if (!icon.isNull())
            item->setIcon(icon);

        thumbnailModel->appendRow(item);
    }
}

void MainWindow::loadSnippetIntoEditor(const QString &id)
{
    Snippet s = snippetMgr->loadSnippet(id);
    if (s.id.isEmpty()) return;

    currentSnippetId = id;
    codeEditor->blockSignals(true);
    codeEditor->setPlainText(s.code);
    codeEditor->blockSignals(false);
    nameEdit->setText(s.name);
    descEdit->setPlainText(s.description);

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
    snippetMgr->saveSnippet(s);
}

void MainWindow::refreshCategoryTree()
{
    categoryModel->clear();
    QStandardItem *rootItem = categoryModel->invisibleRootItem();

    QStandardItem *allItem = new QStandardItem(QStringLiteral("全部"));
    allItem->setData("", Qt::UserRole);
    allItem->setEditable(false);
    rootItem->appendRow(allItem);

    QStringList categories = snippetMgr->getAllCategories();
    for (const QString &cat : categories) {
        buildCategoryTree(rootItem, cat);
    }

    categoryTree->expandAll();
}

void MainWindow::buildCategoryTree(QStandardItem *parent, const QString &path, int depth)
{
    Q_UNUSED(depth);
    if (path.isEmpty()) return;

    int slashPos = path.indexOf('/');
    QString name = (slashPos >= 0) ? path.left(slashPos) : path;
    QString remaining = (slashPos >= 0) ? path.mid(slashPos + 1) : QString();
    QString fullPath = parent->data(Qt::UserRole).toString();
    if (!fullPath.isEmpty()) fullPath += "/";
    fullPath += name;

    QStandardItem *child = nullptr;
    for (int i = 0; i < parent->rowCount(); ++i) {
        QStandardItem *sibling = parent->child(i);
        if (sibling->text() == name) {
            child = sibling;
            break;
        }
    }

    if (!child) {
        child = new QStandardItem(name);
        child->setData(fullPath, Qt::UserRole);
        child->setEditable(false);
        parent->appendRow(child);
    }

    if (!remaining.isEmpty()) {
        buildCategoryTree(child, remaining, depth + 1);
    }
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

QIcon MainWindow::loadThumbnailIcon(const QString &snippetId, bool isPreset) const
{
    Q_UNUSED(isPreset);
    QString pngPath = snippetDataPath(snippetId) + "/preview.png";
    if (QFile::exists(pngPath))
        return QIcon(pngPath);
    return QIcon();
}

void MainWindow::savePreviewData(const QString &pdfPath, const QString &snippetId)
{
    if (snippetId.isEmpty()) return;

    QString basePath = snippetDataPath(snippetId);
    QString previewPdf = basePath + "/preview.pdf";

    if (QFile::exists(previewPdf))
        QFile::remove(previewPdf);
    QFile::copy(pdfPath, previewPdf);

    QProcess pngProc;
    QStringList args;
    args << "-png" << "-r" << "150" << "-singlefile" << pdfPath << (basePath + "/preview");
    pngProc.start("pdftocairo", args);
    pngProc.waitForFinished(5000);
}

void MainWindow::loadPreviewForSnippet(const QString &id)
{
    if (id.isEmpty()) {
        clearPdfPreview();
        return;
    }

    QString previewPdf = snippetDataPath(id) + "/preview.pdf";
    if (QFile::exists(previewPdf)) {
        pdfDoc->load(QFileInfo(previewPdf).absoluteFilePath());
    } else {
        clearPdfPreview();
    }
}

void MainWindow::generateAllPreviews()
{
    QList<Snippet> all = snippetMgr->getAllSnippets();
    all.append(snippetMgr->getAllPresets());

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
        QString code = s.code;

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

        compiler->compile(code, s.templateId, snippetId);
        loop.exec();
    }

    statusBar()->showMessage(QStringLiteral("预览生成完毕: %1 个条目").arg(total), 5000);
    m_batchGenerating = false;
    refreshSearch();
}

void MainWindow::showCategoryContextMenu(const QPoint &pos)
{
    QModelIndex idx = categoryTree->indexAt(pos);
    if (!idx.isValid()) return;
    QStandardItem *item = categoryModel->itemFromIndex(idx);
    if (!item) return;

    QString catData = item->data(Qt::UserRole).toString();
    bool isAll = catData.isEmpty() && item->text() == QStringLiteral("全部");

    QList<QAction*> actions = categoryCtxMenu->actions();
    for (QAction *act : actions) {
        if (act->text() == QStringLiteral("新建大类"))
            act->setVisible(isAll);
        else if (act->text() == QStringLiteral("重命名分类")
              || act->text() == QStringLiteral("删除分类")
              || act->text() == QStringLiteral("新建子分类"))
            act->setVisible(!isAll);
    }

    categoryCtxMenu->popup(categoryTree->viewport()->mapToGlobal(pos));
}

void MainWindow::renameCategoryItem(QStandardItem *item)
{
    if (!item) return;
    QString oldCat = item->data(Qt::UserRole).toString();
    if (oldCat.isEmpty()) return;

    bool ok;
    QString newName = QInputDialog::getText(this, QStringLiteral("重命名分类"),
        QStringLiteral("新名称:"), QLineEdit::Normal, oldCat, &ok);
    if (!ok || newName.isEmpty() || newName == oldCat) return;

    int count = snippetMgr->renameCategory(oldCat, newName);
    statusBar()->showMessage(QStringLiteral("已更新 %1 个片段").arg(count), 3000);
    refreshCategoryTree();
    refreshSearch();
}

void MainWindow::deleteCategoryItem(QStandardItem *item)
{
    if (!item) return;
    QString cat = item->data(Qt::UserRole).toString();
    if (cat.isEmpty()) return;

    int ret = QMessageBox::warning(this, QStringLiteral("删除分类"),
        QStringLiteral("确定删除分类 \"%1\" 及其所有片段吗？").arg(cat),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    int count = snippetMgr->deleteCategory(cat);
    statusBar()->showMessage(QStringLiteral("已删除 %1 个片段").arg(count), 3000);
    currentSnippetId.clear();
    codeEditor->clear();
    nameEdit->clear();
    descEdit->clear();
    clearPdfPreview();
    refreshCategoryTree();
    refreshSearch();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == categoryTree->viewport() && event->type() == QEvent::Drop) {
        QDropEvent *de = static_cast<QDropEvent *>(event);
        const QMimeData *mime = de->mimeData();
        QModelIndex targetIdx = categoryTree->indexAt(de->position().toPoint());
        QString targetCat;
        if (targetIdx.isValid())
            targetCat = targetIdx.data(Qt::UserRole).toString();

        if (mime->hasFormat("application/x-qabstractitemmodeldatalist")) {
            QByteArray data = mime->data("application/x-qabstractitemmodeldatalist");
            QDataStream stream(&data, QIODevice::ReadOnly);
            int row, col;
            QMap<int, QVariant> roleData;
            stream >> row >> col >> roleData;
            QString snippetId = roleData[Qt::UserRole].toString();

            if (!snippetId.isEmpty()) {
                snippetMgr->updateSnippetCategory(snippetId, targetCat);
                refreshCategoryTree();
                refreshSearch();
                statusBar()->showMessage(
                    QStringLiteral("已移动片段到: %1").arg(targetCat.isEmpty() ? "全部" : targetCat),
                    3000);
                de->accept();
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::refreshTemplateCombo()
{
    templateCombo->blockSignals(true);
    templateCombo->clear();
    QString tplDir = SettingsDialog::templateDir();
    QDir d(tplDir);
    QStringList files = d.entryList(QStringList() << "*.tex", QDir::Files);
    for (const QString &f : files) {
        QString id = QFileInfo(f).completeBaseName();
        templateCombo->addItem(id, id);
    }
    templateCombo->setCurrentIndex(0);
    templateCombo->blockSignals(false);
}
