#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>

class SnippetManager;
class LinkManager;
class QLabel;

class SnippetPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    explicit SnippetPropertiesDialog(const QString &snippetId,
                                     SnippetManager *mgr,
                                     QWidget *parent = nullptr,
                                     LinkManager *links = nullptr);

    // Remove the snippet's picture link (file + meta.json field) without any
    // confirmation prompt. Public so tests can exercise the logic without
    // modal dialogs. Returns true when the field was cleared.
    bool deleteLinkFileAndClearField();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSave();
    void onDelete();
    void onExport();
    void onCopyCode();
    void onImportImages();
    void onPasteImage();
    void onViewImage();
    void onReplaceImage();
    void onRemoveImage();
    void onCopyImageName();
    void onRemoveLink();

private:
    void loadSnippet();
    void refreshImageList();
    void updateImageButtonStates();
    QString selectedImageName() const;
    void showImageStatus(const QString &text, bool error = false);
    void updateLinkUi();

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

    QListWidget *m_imageList;
    QLabel *m_imageStatusLabel;
    QPushButton *m_importImagesBtn;
    QPushButton *m_pasteImageBtn;
    QPushButton *m_viewImageBtn;
    QPushButton *m_replaceImageBtn;
    QPushButton *m_removeImageBtn;
    QPushButton *m_copyImageNameBtn;

    QLabel *m_linkStatusLabel;
    QPushButton *m_removeLinkBtn;
    LinkManager *m_linkMgr = nullptr;
};
