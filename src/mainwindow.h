#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QPlainTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private:
    void setupUI();

    QWidget *leftPanel;
    QLineEdit *searchBox;
    QTreeView *categoryTree;
    QListView *thumbnailList;

    QPlainTextEdit *codeEditor;
    QPlainTextEdit *logPanel;

    QWidget *rightPanel;
    QLabel *previewLabel;
    QLineEdit *nameEdit;
    QTextEdit *descEdit;
};
