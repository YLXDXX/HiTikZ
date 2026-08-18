#include "snippet_properties_dialog.h"
#include "snippet_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QKeyEvent>
#include <QImage>
#include <QTemporaryDir>
#include <QDebug>

SnippetPropertiesDialog::SnippetPropertiesDialog(const QString &snippetId,
                                                   SnippetManager *mgr,
                                                   QWidget *parent)
    : QDialog(parent), m_snippetId(snippetId), m_snippetMgr(mgr)
{
    setWindowTitle(QStringLiteral("片段属性"));
    setMinimumSize(480, 640);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *form = new QFormLayout;

    m_nameEdit = new QLineEdit;
    form->addRow(QStringLiteral("名称:"), m_nameEdit);

    m_descEdit = new QTextEdit;
    m_descEdit->setMaximumHeight(80);
    form->addRow(QStringLiteral("简介:"), m_descEdit);

    m_categoryEdit = new QLineEdit;
    form->addRow(QStringLiteral("分类:"), m_categoryEdit);

    m_tagsEdit = new QLineEdit;
    m_tagsEdit->setPlaceholderText(QStringLiteral("标签1, 标签2, ..."));
    form->addRow(QStringLiteral("标签:"), m_tagsEdit);

    m_packagesEdit = new QLineEdit;
    m_packagesEdit->setPlaceholderText(QStringLiteral("如: tikz-3dplot,[european]circuitikz"));
    form->addRow(QStringLiteral("额外宏包:"), m_packagesEdit);

    m_tikzLibrariesEdit = new QLineEdit;
    m_tikzLibrariesEdit->setPlaceholderText(QStringLiteral("如: calc,er,angles"));
    form->addRow(QStringLiteral("TikZ库:"), m_tikzLibrariesEdit);

    m_templateCombo = new QComboBox;
    form->addRow(QStringLiteral("模板:"), m_templateCombo);

    m_compileCmdEdit = new QLineEdit;
    m_compileCmdEdit->setPlaceholderText(
        QStringLiteral("留空使用默认: xelatex -interaction=nonstopmode -halt-on-error -shell-escape"));
    form->addRow(QStringLiteral("编译命令:"), m_compileCmdEdit);

    mainLayout->addLayout(form);

    // ── Image management ──────────────────────────────────────────────────
    QGroupBox *imageGroup = new QGroupBox(QStringLiteral("图片管理"));
    QVBoxLayout *imageLayout = new QVBoxLayout(imageGroup);

    m_imageList = new QListWidget;
    m_imageList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_imageList->setMaximumHeight(130);
    imageLayout->addWidget(m_imageList);

    m_imageStatusLabel = new QLabel;
    m_imageStatusLabel->setWordWrap(true);
    imageLayout->addWidget(m_imageStatusLabel);

    QHBoxLayout *imageBtnRow1 = new QHBoxLayout;
    m_importImagesBtn = new QPushButton(QStringLiteral("导入图片..."));
    m_pasteImageBtn = new QPushButton(QStringLiteral("粘贴图片 (Ctrl+V)"));
    m_viewImageBtn = new QPushButton(QStringLiteral("查看"));
    imageBtnRow1->addWidget(m_importImagesBtn);
    imageBtnRow1->addWidget(m_pasteImageBtn);
    imageBtnRow1->addWidget(m_viewImageBtn);
    imageBtnRow1->addStretch();
    imageLayout->addLayout(imageBtnRow1);

    QHBoxLayout *imageBtnRow2 = new QHBoxLayout;
    m_replaceImageBtn = new QPushButton(QStringLiteral("替换..."));
    m_removeImageBtn = new QPushButton(QStringLiteral("删除"));
    m_copyImageNameBtn = new QPushButton(QStringLiteral("复制文件名"));
    QLabel *imageHint = new QLabel(
        QStringLiteral("代码中引用: \\node {\\includegraphics{a.png}};"));
    imageHint->setStyleSheet(QStringLiteral("color: gray;"));
    imageBtnRow2->addWidget(m_replaceImageBtn);
    imageBtnRow2->addWidget(m_removeImageBtn);
    imageBtnRow2->addWidget(m_copyImageNameBtn);
    imageBtnRow2->addWidget(imageHint);
    imageBtnRow2->addStretch();
    imageLayout->addLayout(imageBtnRow2);

    mainLayout->addWidget(imageGroup);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    m_saveBtn = new QPushButton(QStringLiteral("保存"));
    m_deleteBtn = new QPushButton(QStringLiteral("删除片段"));
    m_exportBtn = new QPushButton(QStringLiteral("导出存档"));
    m_copyCodeBtn = new QPushButton(QStringLiteral("复制代码"));
    QPushButton *cancelBtn = new QPushButton(QStringLiteral("取消"));

    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_copyCodeBtn);
    btnLayout->addWidget(m_exportBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_saveBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onSave);
    connect(m_deleteBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onDelete);
    connect(m_exportBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onExport);
    connect(m_copyCodeBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onCopyCode);

    connect(m_importImagesBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onImportImages);
    connect(m_pasteImageBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onPasteImage);
    connect(m_viewImageBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onViewImage);
    connect(m_replaceImageBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onReplaceImage);
    connect(m_removeImageBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onRemoveImage);
    connect(m_copyImageNameBtn, &QPushButton::clicked, this, &SnippetPropertiesDialog::onCopyImageName);
    connect(m_imageList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) {
        onViewImage();
    });
    // 查看/替换/删除/复制文件名 only make sense with a selected image.
    connect(m_imageList, &QListWidget::currentItemChanged, this,
        [this](QListWidgetItem *, QListWidgetItem *) { updateImageButtonStates(); });
    connect(m_imageList, &QListWidget::itemSelectionChanged, this,
        [this]() { updateImageButtonStates(); });
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    loadSnippet();
}

void SnippetPropertiesDialog::loadSnippet()
{
    Snippet s = m_snippetMgr->loadSnippet(m_snippetId);
    if (s.id.isEmpty()) return;

    m_nameEdit->setText(s.name);
    m_descEdit->setPlainText(s.description);
    m_categoryEdit->setText(s.category);
    m_tagsEdit->setText(s.tags.join(", "));
    m_packagesEdit->setText(s.packages);
    m_tikzLibrariesEdit->setText(s.tikzLibraries);
    m_compileCmdEdit->setText(s.compileCommand);

    m_templateCombo->clear();
    QString tplDirPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) + "/templates";
    QDir tplDir(tplDirPath);
    QStringList files = tplDir.exists()
        ? tplDir.entryList(QStringList() << "*.tex", QDir::Files)
        : QStringList();
    for (const QString &f : files) {
        QString id = QFileInfo(f).completeBaseName();
        m_templateCombo->addItem(id, id);
    }
    if (!s.templateId.isEmpty()) {
        int idx = m_templateCombo->findData(s.templateId);
        if (idx >= 0) m_templateCombo->setCurrentIndex(idx);
    }

    refreshImageList();
}

void SnippetPropertiesDialog::refreshImageList()
{
    m_imageList->clear();
    const Snippet s = m_snippetMgr->loadSnippet(m_snippetId);
    for (const QString &name : s.images) {
        QListWidgetItem *item = new QListWidgetItem(name, m_imageList);
        item->setToolTip(name);
    }
    updateImageButtonStates();
}

void SnippetPropertiesDialog::updateImageButtonStates()
{
    const bool hasSelection = m_imageList->currentItem() != nullptr;
    m_viewImageBtn->setEnabled(hasSelection);
    m_replaceImageBtn->setEnabled(hasSelection);
    m_removeImageBtn->setEnabled(hasSelection);
    m_copyImageNameBtn->setEnabled(hasSelection);
}

QString SnippetPropertiesDialog::selectedImageName() const
{
    QListWidgetItem *item = m_imageList->currentItem();
    if (!item)
        return QString();
    return item->text();
}

void SnippetPropertiesDialog::onSave()
{
    Snippet s = m_snippetMgr->loadSnippet(m_snippetId);
    s.name = m_nameEdit->text();
    s.description = m_descEdit->toPlainText();
    s.category = m_categoryEdit->text();
    s.templateId = m_templateCombo->currentData().toString();
    s.packages = m_packagesEdit->text();
    s.tikzLibraries = m_tikzLibrariesEdit->text();
    s.compileCommand = m_compileCmdEdit->text().trimmed();

    QStringList tags;
    for (const QString &tag : m_tagsEdit->text().split(',')) {
        QString trimmed = tag.trimmed();
        if (!trimmed.isEmpty()) tags.append(trimmed);
    }
    s.tags = tags;

    m_snippetMgr->saveSnippet(s);
    accept();
}

void SnippetPropertiesDialog::onDelete()
{
    Snippet s = m_snippetMgr->loadSnippet(m_snippetId);
    int ret = QMessageBox::question(this, QStringLiteral("确认删除"),
        QStringLiteral("确定要删除片段 \"%1\" 吗？").arg(s.name),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        m_snippetMgr->deleteSnippet(m_snippetId);
        accept();
    }
}

void SnippetPropertiesDialog::onExport()
{
    Snippet s = m_snippetMgr->loadSnippet(m_snippetId);
    QString filePath = QFileDialog::getSaveFileName(this,
        QStringLiteral("导出存档"), s.name + ".tar.gz",
        "TikZ 存档 (*.tar.gz)");
    if (!filePath.isEmpty())
        m_snippetMgr->exportSnippetZip(m_snippetId, filePath);
}

void SnippetPropertiesDialog::onCopyCode()
{
    Snippet s = m_snippetMgr->loadSnippet(m_snippetId);
    QApplication::clipboard()->setText(s.code);
}

void SnippetPropertiesDialog::showImageStatus(const QString &text, bool error)
{
    m_imageStatusLabel->setStyleSheet(error
        ? QStringLiteral("color: #c62828;") : QStringLiteral("color: #2e7d32;"));
    m_imageStatusLabel->setText(text);
}

void SnippetPropertiesDialog::onImportImages()
{
    QStringList exts = SnippetManager::supportedImageExtensions();
    QStringList patterns;
    for (const QString &ext : exts)
        patterns << "*." + ext;
    const QString filter = QStringLiteral("图片文件 (%1);;所有文件 (*)").arg(patterns.join(' '));

    QStringList filePaths = QFileDialog::getOpenFileNames(
        this, QStringLiteral("导入图片"), QString(), filter);
    if (filePaths.isEmpty())
        return;

    QStringList imported;
    QStringList failed;
    for (const QString &path : filePaths) {
        QString name = m_snippetMgr->addImageToSnippet(m_snippetId, path);
        if (name.isEmpty())
            failed << QFileInfo(path).fileName();
        else
            imported << name;
    }

    refreshImageList();

    if (!failed.isEmpty())
        showImageStatus(QStringLiteral("导入失败: ") + failed.join(", "), true);
    else if (!imported.isEmpty())
        showImageStatus(QStringLiteral("已导入: ") + imported.join(", "));
}

void SnippetPropertiesDialog::onPasteImage()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime)
        return;

    QStringList imported;
    QStringList failed;

    if (mime->hasImage()) {
        QImage image = qvariant_cast<QImage>(mime->imageData());
        if (image.isNull()) {
            failed << QStringLiteral("剪贴板图像数据无法读取");
        } else {
            QTemporaryDir tmpDir;
            if (!tmpDir.isValid()) {
                failed << QStringLiteral("无法创建临时文件");
            } else {
                QString tmpPath = tmpDir.path() + "/clipboard.png";
                if (!image.save(tmpPath, "PNG")) {
                    failed << QStringLiteral("剪贴板图像无法保存");
                } else {
                    QString name = m_snippetMgr->addImageToSnippet(m_snippetId, tmpPath);
                    if (name.isEmpty())
                        failed << QStringLiteral("剪贴板图像导入失败");
                    else
                        imported << name;
                }
            }
        }
    } else if (mime->hasUrls()) {
        const QList<QUrl> urls = mime->urls();
        bool anyFile = false;
        for (const QUrl &url : urls) {
            if (!url.isLocalFile())
                continue;
            const QString path = url.toLocalFile();
            if (!SnippetManager::isSupportedImageFile(path))
                continue;
            anyFile = true;
            QString name = m_snippetMgr->addImageToSnippet(m_snippetId, path);
            if (name.isEmpty())
                failed << QFileInfo(path).fileName();
            else
                imported << name;
        }
        if (!anyFile && imported.isEmpty() && failed.isEmpty()) {
            failed << QStringLiteral("剪贴板中没有可用的图片");
        }
    } else {
        failed << QStringLiteral("剪贴板中没有图片数据");
    }

    refreshImageList();

    if (!failed.isEmpty())
        showImageStatus(QStringLiteral("导入失败: ") + failed.join(", "), true);
    else if (!imported.isEmpty())
        showImageStatus(QStringLiteral("已导入: ") + imported.join(", "));
}

void SnippetPropertiesDialog::onViewImage()
{
    const QString name = selectedImageName();
    if (name.isEmpty())
        return;
    const QString path = m_snippetMgr->getSnippetImageDir(m_snippetId) + name;
    if (!QFile::exists(path)) {
        QMessageBox::warning(this, QStringLiteral("查看图片"),
            QStringLiteral("图片文件不存在: %1").arg(path));
        return;
    }
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
        QMessageBox::warning(this, QStringLiteral("查看图片"),
            QStringLiteral("无法调用系统图片查看器打开: %1").arg(path));
    }
}

void SnippetPropertiesDialog::onReplaceImage()
{
    const QString name = selectedImageName();
    if (name.isEmpty())
        return;

    const QString filter = QStringLiteral("图片文件 (*.png *.jpg *.jpeg *.tif *.tiff *.bmp *.gif *.webp *.pdf);;所有文件 (*)");
    const QString filePath = QFileDialog::getOpenFileName(
        this, QStringLiteral("替换图片 %1").arg(name), QString(), filter);
    if (filePath.isEmpty())
        return;

    const QString newName = m_snippetMgr->replaceSnippetImage(m_snippetId, name, filePath);
    refreshImageList();
    if (newName.isEmpty()) {
        showImageStatus(QStringLiteral("替换图片 %1 失败").arg(name), true);
    } else if (newName != name) {
        showImageStatus(QStringLiteral("图片已替换为 %1（扩展名已更新，请同步检查代码中的引用）").arg(newName));
    } else {
        showImageStatus(QStringLiteral("图片 %1 已替换").arg(newName));
    }
}

void SnippetPropertiesDialog::onRemoveImage()
{
    const QString name = selectedImageName();
    if (name.isEmpty())
        return;

    const int ret = QMessageBox::question(this, QStringLiteral("删除图片"),
        QStringLiteral("确定要删除图片 \"%1\" 吗？\n代码中对它的引用将无法编译，请同步修改代码。").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
        return;

    m_snippetMgr->removeSnippetImage(m_snippetId, name);
    refreshImageList();
    showImageStatus(QStringLiteral("图片 %1 已删除（代码中的引用请同步移除）").arg(name));
}

void SnippetPropertiesDialog::onCopyImageName()
{
    const QString name = selectedImageName();
    if (name.isEmpty())
        return;
    QApplication::clipboard()->setText(name);
}

void SnippetPropertiesDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Paste)) {
        QWidget *f = focusWidget();
        if (!qobject_cast<QLineEdit*>(f) && !qobject_cast<QTextEdit*>(f)
            && !qobject_cast<QComboBox*>(f) && !qobject_cast<QAbstractSpinBox*>(f)) {
            onPasteImage();
            event->accept();
            return;
        }
    }
    QDialog::keyPressEvent(event);
}
