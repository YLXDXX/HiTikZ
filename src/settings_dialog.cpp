#include "settings_dialog.h"
#include "latex_compiler.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("设置"));
    setMinimumSize(600, 450);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout;
    xelatexPathEdit = new QLineEdit;
    pdftocairoPathEdit = new QLineEdit;
    texInputsEdit = new QLineEdit;
    texInputsEdit->setPlaceholderText(QStringLiteral("额外的TEXINPUTS路径，用冒号分隔"));
    pngDpiSpin = new QSpinBox;
    pngDpiSpin->setRange(72, 1200);
    pngDpiSpin->setValue(300);

    formLayout->addRow(QStringLiteral("xelatex 路径:"), xelatexPathEdit);
    formLayout->addRow(QStringLiteral("pdftocairo 路径:"), pdftocairoPathEdit);
    formLayout->addRow(QStringLiteral("TEXINPUTS:"), texInputsEdit);
    formLayout->addRow(QStringLiteral("PNG DPI:"), pngDpiSpin);
    mainLayout->addLayout(formLayout);

    QGroupBox *shortcutGroup = new QGroupBox(QStringLiteral("快捷键设置"));
    QFormLayout *shortcutLayout = new QFormLayout(shortcutGroup);
    copyCodeShortcutEdit = new QKeySequenceEdit;
    copyPngShortcutEdit = new QKeySequenceEdit;
    copySvgShortcutEdit = new QKeySequenceEdit;
    shortcutLayout->addRow(QStringLiteral("复制代码:"), copyCodeShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("复制PNG:"), copyPngShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("复制SVG:"), copySvgShortcutEdit);
    globalHotkeyEdit = new QKeySequenceEdit;
    shortcutLayout->addRow(QStringLiteral("全局快捷键:"), globalHotkeyEdit);
    mainLayout->addWidget(shortcutGroup);

    QGroupBox *actionsGroup = new QGroupBox(QStringLiteral("工具"));
    QVBoxLayout *actionsLayout = new QVBoxLayout(actionsGroup);
    QPushButton *genPreviewBtn = new QPushButton(QStringLiteral("生成所有预览"));
    QPushButton *resetBtn = new QPushButton(QStringLiteral("恢复出厂设置"));
    resetBtn->setStyleSheet("QPushButton { color: red; }");
    actionsLayout->addWidget(genPreviewBtn);
    actionsLayout->addWidget(resetBtn);
    mainLayout->addWidget(actionsGroup);

    connect(genPreviewBtn, &QPushButton::clicked, this, [this]() {
        auto *mw = qobject_cast<MainWindow*>(parentWidget());
        if (mw) {
            mw->generateAllPreviews();
            QMessageBox::information(this, QStringLiteral("完成"),
                QStringLiteral("所有预览生成完毕。"));
        }
    });

    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this, QStringLiteral("⚠ 危险操作"),
            QStringLiteral("这将永久删除所有用户创建的片段和修改！\n\n"
                           "请在下方输入 \"确定重置\" 来确认此操作："),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;

        bool ok;
        QString confirm = QInputDialog::getText(this, QStringLiteral("确认重置"),
            QStringLiteral("请输入 \"确定重置\" 以确认:"),
            QLineEdit::Normal, "", &ok);
        if (!ok || confirm != QStringLiteral("确定重置")) {
            QMessageBox::information(this, QStringLiteral("取消"),
                QStringLiteral("重置操作已取消。"));
            return;
        }

        auto *mw = qobject_cast<MainWindow*>(parentWidget());
        if (mw) {
            mw->factoryReset();
            QMessageBox::information(this, QStringLiteral("完成"),
                QStringLiteral("已恢复到出厂设置，建议重启程序。"));

            saveSettings();
            saveTemplateContent();
            accept();
        }
    });

    QLabel *templateLabel = new QLabel(QStringLiteral("LaTeX 模板管理:"));
    mainLayout->addWidget(templateLabel);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    QWidget *listPanel = new QWidget;
    QVBoxLayout *listLayout = new QVBoxLayout(listPanel);
    listLayout->setContentsMargins(0, 0, 0, 0);
    templateListWidget = new QListWidget;
    listLayout->addWidget(templateListWidget);

    QHBoxLayout *tplBtnLayout = new QHBoxLayout;
    QPushButton *newTplBtn = new QPushButton(QStringLiteral("+"));
    newTplBtn->setFixedWidth(30);
    QPushButton *delTplBtn = new QPushButton(QStringLiteral("-"));
    delTplBtn->setFixedWidth(30);
    tplBtnLayout->addStretch();
    tplBtnLayout->addWidget(newTplBtn);
    tplBtnLayout->addWidget(delTplBtn);
    listLayout->addLayout(tplBtnLayout);

    connect(newTplBtn, &QPushButton::clicked, this, &SettingsDialog::createTemplate);
    connect(delTplBtn, &QPushButton::clicked, this, &SettingsDialog::deleteTemplate);

    QWidget *editPanel = new QWidget;
    QVBoxLayout *editLayout = new QVBoxLayout(editPanel);
    editLayout->setContentsMargins(0, 0, 0, 0);
    templateEdit = new QPlainTextEdit;
    templateEdit->setFont(QFont("monospace", 10));
    editLayout->addWidget(templateEdit);
    QPushButton *saveTemplateBtn = new QPushButton(QStringLiteral("保存模板"));
    editLayout->addWidget(saveTemplateBtn);

    splitter->addWidget(listPanel);
    splitter->addWidget(editPanel);
    mainLayout->addWidget(splitter, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        saveSettings();
        saveTemplateContent();
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(saveTemplateBtn, &QPushButton::clicked, this, &SettingsDialog::saveTemplateContent);

    connect(templateListWidget, &QListWidget::currentTextChanged,
        this, [this](const QString &) {
            loadTemplateContent();
        });

    loadSettings();
    loadTemplateList();
}

void SettingsDialog::loadSettings()
{
    QSettings settings("HiTikZ", "TikzManager");
    xelatexPathEdit->setText(settings.value("xelatex/path", "xelatex").toString());
    pdftocairoPathEdit->setText(settings.value("pdftocairo/path", "pdftocairo").toString());
    texInputsEdit->setText(settings.value("paths/texinputs", "").toString());
    pngDpiSpin->setValue(settings.value("png/dpi", 300).toInt());
    copyCodeShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/copyCode", "Ctrl+Shift+C").toString()));
    copyPngShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/copyPng", "Ctrl+Shift+P").toString()));
    copySvgShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/copySvg", "Ctrl+Shift+S").toString()));
    globalHotkeyEdit->setKeySequence(QKeySequence(settings.value("shortcuts/globalHotkey", "Ctrl+Alt+T").toString()));
}

void SettingsDialog::saveSettings()
{
    QSettings settings("HiTikZ", "TikzManager");
    settings.setValue("xelatex/path", xelatexPathEdit->text());
    settings.setValue("pdftocairo/path", pdftocairoPathEdit->text());
    settings.setValue("paths/texinputs", texInputsEdit->text());
    settings.setValue("png/dpi", pngDpiSpin->value());
    settings.setValue("shortcuts/copyCode", copyCodeShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/copyPng", copyPngShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/copySvg", copySvgShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/globalHotkey", globalHotkeyEdit->keySequence().toString());
}

void SettingsDialog::applyToCompiler(LatexCompiler *compiler)
{
    QSettings settings("HiTikZ", "TikzManager");
    compiler->setXelatexPath(settings.value("xelatex/path", "xelatex").toString());
    compiler->setPdfToCairoPath(settings.value("pdftocairo/path", "pdftocairo").toString());
    compiler->setTexInputs(settings.value("paths/texinputs", "").toString());
    compiler->setTemplateDir(templateDir());
}

QString SettingsDialog::templateDir()
{
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dataLocation + "/templates/";
}

void SettingsDialog::ensureTemplatesCopied(const QString &resourceTemplateDir)
{
    QString destDir = templateDir();
    QDir().mkpath(destDir);

    QDir resDir(resourceTemplateDir);
    if (!resDir.exists()) return;

    QStringList templates = resDir.entryList(QStringList() << "*.tex", QDir::Files);
    for (const QString &tpl : templates) {
        QString destPath = destDir + tpl;
        if (!QFile::exists(destPath)) {
            QFile::copy(resourceTemplateDir + "/" + tpl, destPath);
        }
    }
}

void SettingsDialog::loadTemplateList()
{
    templateListWidget->clear();
    QString dir = templateDir();
    QDir d(dir);
    QStringList templates = d.entryList(QStringList() << "*.tex", QDir::Files);
    for (const QString &tpl : templates) {
        QFileInfo fi(tpl);
        templateListWidget->addItem(fi.baseName());
    }
    if (templateListWidget->count() > 0) {
        templateListWidget->setCurrentRow(0);
    }
}

void SettingsDialog::loadTemplateContent()
{
    QListWidgetItem *item = templateListWidget->currentItem();
    if (!item) {
        templateEdit->clear();
        currentTemplateFile.clear();
        return;
    }

    currentTemplateFile = templateDir() + item->text() + ".tex";
    QFile file(currentTemplateFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        templateEdit->setPlainText(QString::fromUtf8(file.readAll()));
        file.close();
    }
}

void SettingsDialog::saveTemplateContent()
{
    QListWidgetItem *item = templateListWidget->currentItem();
    if (!item) return;

    currentTemplateFile = templateDir() + item->text() + ".tex";
    QFile file(currentTemplateFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        file.write(templateEdit->toPlainText().toUtf8());
        file.close();
    }
}

void SettingsDialog::createTemplate()
{
    bool ok;
    QString name = QInputDialog::getText(this, QStringLiteral("新建模板"),
        QStringLiteral("模板名称 (不含扩展名):"), QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    QString path = templateDir() + name + ".tex";
    if (QFile::exists(path)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
            QStringLiteral("模板 \"%1\" 已存在").arg(name));
        return;
    }

    QString defaultContent = QStringLiteral(
        "\\documentclass[tikz, border=5pt]{standalone}\n"
        "\\usepackage{tikz}\n"
        "\\usepackage{xcolor}\n"
        "\\usepackage{ctex}\n"
        "\\begin{document}\n"
        "%%% TIKZ_CODE_HERE %%%\n"
        "\\end{document}\n"
    );

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(defaultContent.toUtf8());
        file.close();
    }

    loadTemplateList();
    int idx = templateListWidget->findItems(name, Qt::MatchExactly).value(0) ?
        templateListWidget->row(templateListWidget->findItems(name, Qt::MatchExactly).first()) : 0;
    templateListWidget->setCurrentRow(idx >= 0 ? idx : 0);
}

void SettingsDialog::deleteTemplate()
{
    QListWidgetItem *item = templateListWidget->currentItem();
    if (!item) return;

    QString name = item->text();
    int ret = QMessageBox::warning(this, QStringLiteral("删除模板"),
        QStringLiteral("确定删除模板 \"%1\" 吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    QString path = templateDir() + name + ".tex";
    QFile::remove(path);
    loadTemplateList();
}
