#include "mainwindow.h"
#include "search_panel.h"
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
                searchPanel->setFocus();
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
    QAction *importAct = toolBar->addAction(QStringLiteral("导入(导出)存档"));
    QAction *exportAct = toolBar->addAction(QStringLiteral("存档"));
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
    rightLayout->addWidget(new QLabel(QStringLiteral("标签 (逗号分隔):")));
    tagsEdit = new QLineEdit;
    tagsEdit->setPlaceholderText(QStringLiteral("标签1, 标签2, ..."));
    rightLayout->addWidget(tagsEdit);
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

    connect(exportAct, &QAction::triggered, this, [this]() {
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

    connect(searchPanel, &SearchPanel::snippetDeleteRequested,
        this, &MainWindow::handleThumbnailDelete);

    connect(searchPanel, &SearchPanel::snippetExportRequested,
        this, &MainWindow::handleThumbnailExport);

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
            setFormattedLog(log);
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
    QStringList tags;
    for (const QString &tag : tagsEdit->text().split(',')) {
        QString trimmed = tag.trimmed();
        if (!trimmed.isEmpty())
            tags.append(trimmed);
    }
    s.tags = tags;
    snippetMgr->saveSnippet(s);
}

void MainWindow::handleThumbnailDelete(const QString &id)
{
    if (id.isEmpty()) return;
    Snippet s = snippetMgr->loadSnippet(id);
    int ret = QMessageBox::question(this, QStringLiteral("确认删除"),
        QStringLiteral("确定要删除片段 \"%1\" 吗？").arg(s.name),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    snippetMgr->deleteSnippet(id);
    if (currentSnippetId == id) {
        currentSnippetId.clear();
        codeEditor->clear();
        nameEdit->clear();
        descEdit->clear();
        tagsEdit->clear();
        clearPdfPreview();
    }
    searchPanel->refreshCategoryTree();
    searchPanel->refreshSearch();
    statusBar()->showMessage(QStringLiteral("已删除片段"), 3000);
}

void MainWindow::handleThumbnailExport(const QString &id)
{
    if (id.isEmpty()) return;
    Snippet s = snippetMgr->loadSnippet(id);
    QString filePath = QFileDialog::getSaveFileName(this,
        QStringLiteral("导出存档"), s.name + ".tar.gz",
        "TikZ 存档 (*.tar.gz)");
    if (!filePath.isEmpty()) {
        if (snippetMgr->exportSnippetZip(id, filePath)) {
            statusBar()->showMessage(QStringLiteral("导出成功"), 3000);
        } else {
            QMessageBox::warning(this, QStringLiteral("导出失败"),
                QStringLiteral("无法导出该片段。"));
        }
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

void MainWindow::savePreviewData(const QString &pdfPath, const QString &snippetId)
{
    if (snippetId.isEmpty()) return;

    QString basePath = snippetDataPath(snippetId);
    QString previewPdf = basePath + "/preview.pdf";

    if (QFile::exists(previewPdf))
        QFile::remove(previewPdf);
    QFile::copy(pdfPath, previewPdf);

    QProcess *pngProc = new QProcess(this);
    connect(pngProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        pngProc, &QProcess::deleteLater);
    QStringList args;
    args << "-png" << "-r" << "150" << "-singlefile" << pdfPath << (basePath + "/preview");
    pngProc->start("pdftocairo", args);
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

        compiler->compile(code, s.templateId, snippetId);
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
