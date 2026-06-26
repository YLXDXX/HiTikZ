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
#include <QStackedWidget>

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
    void savePreviewPng(const QString &pdfPath, const QString &snippetId);
    void loadPreviewForSnippet(const QString &id);
    void clearPreview();
    void generateAllPreviews();

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
    QStackedWidget *previewStack;
    QLabel *previewImageLabel;
    QPdfView *pdfView;
    QPdfDocument *pdfDoc;
    QLineEdit *nameEdit;
    QTextEdit *descEdit;
    QPushButton *compileBtn;
    QPushButton *saveBtn;

    QScrollArea *paramsScrollArea;
    QWidget *paramsWidget;
    QVBoxLayout *paramsLayout;
    QPushButton *applyParamsBtn;
    QList<ParamInfo> currentParams;

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    QString currentSnippetId;
};
