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

class SnippetManager;
class LatexCompiler;
class CodeEditor;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void setupConnections();
    void refreshSearch();
    void loadSnippetIntoEditor(const QString &id);
    void saveCurrentSnippet();
    void refreshCategoryTree();
    void onCurrentSnippetChanged();

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
    QLabel *previewLabel;
    QLineEdit *nameEdit;
    QTextEdit *descEdit;
    QPushButton *compileBtn;
    QPushButton *saveBtn;

    QString currentSnippetId;
};
