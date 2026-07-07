#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>

class SnippetManager;

class SnippetPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    explicit SnippetPropertiesDialog(const QString &snippetId,
                                     SnippetManager *mgr,
                                     QWidget *parent = nullptr);

private slots:
    void onSave();
    void onDelete();
    void onExport();
    void onCopyCode();

private:
    void loadSnippet();

    QString m_snippetId;
    SnippetManager *m_snippetMgr;

    QLineEdit *m_nameEdit;
    QTextEdit *m_descEdit;
    QLineEdit *m_categoryEdit;
    QLineEdit *m_tagsEdit;
    QLineEdit *m_packagesEdit;
    QLineEdit *m_tikzLibrariesEdit;
    QComboBox *m_templateCombo;
    QLineEdit *m_compileCmdEdit;
    QPushButton *m_saveBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_copyCodeBtn;
};
