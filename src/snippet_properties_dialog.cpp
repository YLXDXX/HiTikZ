#include "snippet_properties_dialog.h"
#include "snippet_manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

SnippetPropertiesDialog::SnippetPropertiesDialog(const QString &snippetId,
                                                   SnippetManager *mgr,
                                                   QWidget *parent)
    : QDialog(parent), m_snippetId(snippetId), m_snippetMgr(mgr)
{
    setWindowTitle(QStringLiteral("片段属性"));
    setMinimumSize(450, 480);

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
