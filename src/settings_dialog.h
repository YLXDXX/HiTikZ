#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QSettings>
#include <QStringList>
#include <QKeySequenceEdit>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    static void applyToCompiler(class LatexCompiler *compiler);
    static QString templateDir();
    static void ensureTemplatesCopied(const QString &resourceTemplateDir);

private:
    void loadSettings();
    void saveSettings();
    void loadTemplateList();
    void loadTemplateContent();
    void saveTemplateContent();
    void createTemplate();
    void deleteTemplate();

    QLineEdit *xelatexPathEdit;
    QLineEdit *pdftocairoPathEdit;
    QLineEdit *texInputsEdit;
    QSpinBox *pngDpiSpin;
    QKeySequenceEdit *copyCodeShortcutEdit;
    QKeySequenceEdit *copyPngShortcutEdit;
    QKeySequenceEdit *copySvgShortcutEdit;
    QListWidget *templateListWidget;
    QPlainTextEdit *templateEdit;

    QString currentTemplateFile;
};
