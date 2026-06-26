#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
}

void MainWindow::setupUI() {
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);

    leftPanel = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    searchBox = new QLineEdit;
    searchBox->setPlaceholderText(QStringLiteral("搜索名称或简介..."));
    categoryTree = new QTreeView;
    thumbnailList = new QListView;
    leftLayout->addWidget(searchBox);
    leftLayout->addWidget(categoryTree, 1);
    leftLayout->addWidget(thumbnailList, 2);

    QSplitter *centerSplitter = new QSplitter(Qt::Vertical);
    codeEditor = new QPlainTextEdit;
    codeEditor->setFont(QFont("monospace", 10));
    logPanel = new QPlainTextEdit;
    logPanel->setReadOnly(true);
    centerSplitter->addWidget(codeEditor);
    centerSplitter->addWidget(logPanel);
    centerSplitter->setSizes({600, 100});

    rightPanel = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    previewLabel = new QLabel(QStringLiteral("预览区"));
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(300, 300);
    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText(QStringLiteral("名称"));
    descEdit = new QTextEdit;
    descEdit->setPlaceholderText(QStringLiteral("简介"));
    rightLayout->addWidget(previewLabel, 3);
    rightLayout->addWidget(nameEdit);
    rightLayout->addWidget(descEdit, 1);

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(centerSplitter);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({250, 600, 350});

    setCentralWidget(mainSplitter);
}
