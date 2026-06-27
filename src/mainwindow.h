#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QPlainTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPdfDocument>
#include <QPdfView>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QComboBox>
#include <QTimer>

#ifdef HAS_QHOTKEY
#include <QHotkey>
#endif

class SnippetManager;
class LatexCompiler;
class CodeEditor;

struct ParamInfo {
    QString name;
    QString defaultValue;
    QLineEdit *edit;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void setupConnections();
    void refreshSearch();
    void loadSnippetIntoEditor(const QString &id);
    void saveCurrentSnippet();
    void refreshCategoryTree();
    void onCurrentSnippetChanged();
    void jumpToErrorLine(const QString &logText);
    void parseParams();
    void clearParams();
    QString applyParams(const QString &code);
    void savePreviewData(const QString &pdfPath, const QString &snippetId);
    void loadPreviewForSnippet(const QString &id);
    void clearPdfPreview();
    void generateAllPreviews();
    void setFormattedLog(const QString &log);
    void handleLogDoubleClick();

    QString snippetDataPath(const QString &id) const;
    QIcon loadThumbnailIcon(const QString &snippetId, bool isPreset) const;

    void buildCategoryTree(QStandardItem *parent, const QString &path, int depth = 0);
    void showCategoryContextMenu(const QPoint &pos);
    void renameCategoryItem(QStandardItem *item);
    void deleteCategoryItem(QStandardItem *item);
    void refreshTemplateCombo();

    SnippetManager *snippetMgr;
    LatexCompiler *compiler;
    QStandardItemModel *thumbnailModel;
    QStandardItemModel *categoryModel;

    QWidget *leftPanel;
    QLineEdit *searchBox;
    QTreeView *categoryTree;
    QListView *thumbnailList;

    CodeEditor *codeEditor;
    QPlainTextEdit *logPanel;

    QWidget *rightPanel;
    QPdfView *pdfView;
    QPdfDocument *pdfDoc;
    QLineEdit *nameEdit;
    QTextEdit *descEdit;
    QComboBox *templateCombo;
    QPushButton *compileBtn;
    QPushButton *saveBtn;

    QScrollArea *paramsScrollArea;
    QWidget *paramsWidget;
    QVBoxLayout *paramsLayout;
    QPushButton *applyParamsBtn;
    QList<ParamInfo> currentParams;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QMenu *categoryCtxMenu;

    QString currentSnippetId;
    bool m_batchGenerating = false;
    QTimer *searchDebounceTimer = nullptr;
};
