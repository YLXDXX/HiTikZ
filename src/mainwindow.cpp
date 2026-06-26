#include "mainwindow.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "code_editor.h"
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentSnippetId("")
{
    snippetMgr = new SnippetManager(this);
    compiler = new LatexCompiler(this);

    setupUI();
    setupConnections();
    refreshCategoryTree();
    refreshSearch();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle(QStringLiteral("HiTikZ - TikZ 代码合集管理器"));

    QToolBar *toolBar = addToolBar(QStringLiteral("工具栏"));
    QAction *newAct = toolBar->addAction(QStringLiteral("新建片段"));
    QAction *deleteAct = toolBar->addAction(QStringLiteral("删除片段"));
    QAction *importAct = toolBar->addAction(QStringLiteral("导入ZIP"));
    QAction *exportAct = toolBar->addAction(QStringLiteral("导出ZIP"));

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
    categoryModel = new QStandardItemModel(this);
    categoryTree->setModel(categoryModel);

    thumbnailList = new QListView;
    thumbnailList->setViewMode(QListView::IconMode);
    thumbnailList->setIconSize(QSize(96, 96));
    thumbnailList->setGridSize(QSize(120, 130));
    thumbnailList->setResizeMode(QListView::Adjust);
    thumbnailList->setWordWrap(true);
    thumbnailModel = new QStandardItemModel(this);
    thumbnailList->setModel(thumbnailModel);

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
    pdfView = new QPdfView(this);
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
    rightLayout->addWidget(compileBtn);
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

            QList<SearchResult> results = snippetMgr->searchSnippets("");
            for (const SearchResult &r : results) {
                if (!category.isEmpty() && r.snippet.category != category)
                    continue;
                QStandardItem *item = new QStandardItem(r.snippet.name);
                item->setData(r.snippet.id, Qt::UserRole);
                item->setToolTip(r.snippet.description);
                thumbnailModel->appendRow(item);
            }
        });

    connect(compileBtn, &QPushButton::clicked, this, [this]() {
        if (currentSnippetId.isEmpty()) return;
        saveCurrentSnippet();
        Snippet s = snippetMgr->loadSnippet(currentSnippetId);
        logPanel->clear();
        compiler->compile(s.code, s.templateId, currentSnippetId);
    });

    connect(saveBtn, &QPushButton::clicked, this, [this]() {
        saveCurrentSnippet();
        refreshSearch();
    });

    connect(compiler, &LatexCompiler::compilationFinished,
        this, [this](bool success, const QString &pdfPath, const QString &log) {
            logPanel->setPlainText(log);
            if (success) {
                pdfDoc->load(QFileInfo(pdfPath).absoluteFilePath());
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
        QStandardItem *item = new QStandardItem(r.snippet.name);
        item->setData(r.snippet.id, Qt::UserRole);
        item->setToolTip(QString("%1 (分数: %2)\n%3")
            .arg(r.snippet.name).arg(r.score).arg(r.snippet.description));
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
    rootItem->appendRow(allItem);

    QStringList categories = snippetMgr->getAllCategories();
    for (const QString &cat : categories) {
        QStandardItem *catItem = new QStandardItem(cat);
        catItem->setData(cat, Qt::UserRole);
        rootItem->appendRow(catItem);
    }
}

void MainWindow::onCurrentSnippetChanged()
{
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
