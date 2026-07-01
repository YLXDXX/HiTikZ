#include "settings_dialog.h"
#include "latex_compiler.h"
#include "mainwindow.h"
#include "snippet_manager.h"
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
#include <QCheckBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("设置"));
    setMinimumSize(750, 800);
    resize(800, 900);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout;
    xelatexPathEdit = new QLineEdit;
    pdftocairoPathEdit = new QLineEdit;
    inkscapePathEdit = new QLineEdit;
    svgToolCombo = new QComboBox;
    svgToolCombo->addItem("pdftocairo", "pdftocairo");
    svgToolCombo->addItem("inkscape", "inkscape");
    texInputsEdit = new QLineEdit;
    texInputsEdit->setPlaceholderText(QStringLiteral("额外的TEXINPUTS路径，用冒号分隔"));
    pngDpiSpin = new QSpinBox;
    pngDpiSpin->setRange(72, 1200);
    pngDpiSpin->setValue(300);
    editorFontSizeSpin = new QSpinBox;
    editorFontSizeSpin->setRange(8, 48);
    editorFontSizeSpin->setValue(10);
    uiFontSizeSpin = new QSpinBox;
    uiFontSizeSpin->setRange(8, 48);
    uiFontSizeSpin->setValue(10);

    formLayout->addRow(QStringLiteral("xelatex 命令:"), xelatexPathEdit);
    formLayout->addRow(QStringLiteral("pdftocairo 命令:"), pdftocairoPathEdit);
    formLayout->addRow(QStringLiteral("inkscape 命令:"), inkscapePathEdit);
    formLayout->addRow(QStringLiteral("SVG 转换工具:"), svgToolCombo);
    formLayout->addRow(QStringLiteral("额外环境变量:"), texInputsEdit);
    formLayout->addRow(QStringLiteral("PNG DPI:"), pngDpiSpin);
    formLayout->addRow(QStringLiteral("代码字体大小:"), editorFontSizeSpin);
    formLayout->addRow(QStringLiteral("界面字体大小:"), uiFontSizeSpin);
    mainLayout->addLayout(formLayout);

    QGroupBox *behaviorGroup = new QGroupBox(QStringLiteral("行为设置"));
    QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorGroup);
    autoCompileOnSaveCheck = new QCheckBox(QStringLiteral("保存后自动编译"));
    behaviorLayout->addWidget(autoCompileOnSaveCheck);
    mainLayout->addWidget(behaviorGroup);

    QGroupBox *shortcutGroup = new QGroupBox(QStringLiteral("快捷键设置"));
    QFormLayout *shortcutLayout = new QFormLayout(shortcutGroup);

    copyCodeShortcutEdit = new QKeySequenceEdit;
    copyPngShortcutEdit = new QKeySequenceEdit;
    copySvgShortcutEdit = new QKeySequenceEdit;
    compileShortcutEdit = new QKeySequenceEdit;
    applyParamsShortcutEdit = new QKeySequenceEdit;
    saveShortcutEdit = new QKeySequenceEdit;
    closeTabShortcutEdit = new QKeySequenceEdit;
    globalHotkeyEdit = new QKeySequenceEdit;

    QList<QKeySequenceEdit*> allEdits = {copyCodeShortcutEdit, copyPngShortcutEdit,
        copySvgShortcutEdit, compileShortcutEdit, applyParamsShortcutEdit,
        saveShortcutEdit, closeTabShortcutEdit, globalHotkeyEdit};
    for (QKeySequenceEdit *edit : allEdits) {
        edit->setClearButtonEnabled(true);
    }

    shortcutLayout->addRow(QStringLiteral("复制代码:"), copyCodeShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("复制PNG:"), copyPngShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("复制SVG:"), copySvgShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("编译预览:"), compileShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("应用参数:"), applyParamsShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("保存:"), saveShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("关闭标签页:"), closeTabShortcutEdit);
    shortcutLayout->addRow(QStringLiteral("全局快捷键:"), globalHotkeyEdit);
    mainLayout->addWidget(shortcutGroup);

    QGroupBox *actionsGroup = new QGroupBox(QStringLiteral("工具"));
    QVBoxLayout *actionsLayout = new QVBoxLayout(actionsGroup);
    QPushButton *genPreviewBtn = new QPushButton(QStringLiteral("生成所有预览"));
    QPushButton *resetBtn = new QPushButton(QStringLiteral("重置所有内容"));
    resetBtn->setStyleSheet("QPushButton { color: red; }");
    actionsLayout->addWidget(genPreviewBtn);
    actionsLayout->addWidget(resetBtn);
    mainLayout->addWidget(actionsGroup);

    connect(genPreviewBtn, &QPushButton::clicked, this, [this]() {
        auto *mw = qobject_cast<MainWindow*>(parentWidget());
        if (mw) {
            mw->generateAllPreviews();
            accept();
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
    inkscapePathEdit->setText(settings.value("inkscape/path", "inkscape").toString());
    svgToolCombo->setCurrentIndex(svgToolCombo->findData(settings.value("svg/tool", "pdftocairo").toString()));
    texInputsEdit->setText(settings.value("paths/texinputs", "").toString());
    pngDpiSpin->setValue(settings.value("png/dpi", 300).toInt());
    editorFontSizeSpin->setValue(settings.value("editor/fontSize", 10).toInt());
    uiFontSizeSpin->setValue(settings.value("ui/fontSize", 10).toInt());
    copyCodeShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/copyCode", "").toString()));
    copyPngShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/copyPng", "").toString()));
    copySvgShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/copySvg", "").toString()));
    compileShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/compile", "F6").toString()));
    applyParamsShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/applyParams", "").toString()));
    saveShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/save", "Ctrl+S").toString()));
    closeTabShortcutEdit->setKeySequence(QKeySequence(settings.value("shortcuts/closeTab", "Ctrl+W").toString()));
    globalHotkeyEdit->setKeySequence(QKeySequence(settings.value("shortcuts/globalHotkey", "").toString()));
    autoCompileOnSaveCheck->setChecked(settings.value("behavior/autoCompileOnSave", true).toBool());
}

void SettingsDialog::saveSettings()
{
    QSettings settings("HiTikZ", "TikzManager");
    settings.setValue("xelatex/path", xelatexPathEdit->text());
    settings.setValue("pdftocairo/path", pdftocairoPathEdit->text());
    settings.setValue("inkscape/path", inkscapePathEdit->text());
    settings.setValue("svg/tool", svgToolCombo->currentData().toString());
    settings.setValue("paths/texinputs", texInputsEdit->text());
    settings.setValue("png/dpi", pngDpiSpin->value());
    settings.setValue("editor/fontSize", editorFontSizeSpin->value());
    settings.setValue("ui/fontSize", uiFontSizeSpin->value());
    settings.setValue("shortcuts/copyCode", copyCodeShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/copyPng", copyPngShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/copySvg", copySvgShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/compile", compileShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/applyParams", applyParamsShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/save", saveShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/closeTab", closeTabShortcutEdit->keySequence().toString());
    settings.setValue("shortcuts/globalHotkey", globalHotkeyEdit->keySequence().toString());
    settings.setValue("behavior/autoCompileOnSave", autoCompileOnSaveCheck->isChecked());
}

void SettingsDialog::applyToCompiler(LatexCompiler *compiler)
{
    QSettings settings("HiTikZ", "TikzManager");
    compiler->setXelatexPath(settings.value("xelatex/path", "xelatex").toString());
    compiler->setPdfToCairoPath(settings.value("pdftocairo/path", "pdftocairo").toString());
    compiler->setInkscapePath(settings.value("inkscape/path", "inkscape").toString());
    compiler->setSvgTool(settings.value("svg/tool", "pdftocairo").toString());
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
            if (!QFile::copy(resourceTemplateDir + "/" + tpl, destPath))
                qWarning() << "Failed to copy template:" << tpl;
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
    const QList<QListWidgetItem *> found = templateListWidget->findItems(name, Qt::MatchExactly);
    int idx = found.isEmpty() ? 0 : templateListWidget->row(found.first());
    templateListWidget->setCurrentRow(idx >= 0 ? idx : 0);
}

void SettingsDialog::setSnippetManager(SnippetManager *mgr)
{
    m_snippetMgr = mgr;
}

void SettingsDialog::deleteTemplate()
{
    QListWidgetItem *item = templateListWidget->currentItem();
    if (!item) return;

    QString name = item->text();

    if (m_snippetMgr) {
        QList<Snippet> all = m_snippetMgr->getAllSnippets(true);
        QList<Snippet> presets = m_snippetMgr->getAllPresets(true);
        all.append(presets);
        QStringList usingSnippets;
        for (const Snippet &s : all) {
            if (s.templateId == name)
                usingSnippets.append(s.name);
        }
        if (!usingSnippets.isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("无法删除"),
                QStringLiteral("模板 \"%1\" 正在被以下片段使用，无法删除:\n\n%2")
                    .arg(name, usingSnippets.join("\n")));
            return;
        }
    }

    int ret = QMessageBox::warning(this, QStringLiteral("删除模板"),
        QStringLiteral("确定删除模板 \"%1\" 吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    QString path = templateDir() + name + ".tex";
    QFile::remove(path);
    loadTemplateList();
}
