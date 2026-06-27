#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QTreeView>
#include <QListView>
#include <QStandardItemModel>
#include <QMenu>
#include <QTimer>

class SnippetManager;

class SearchPanel : public QWidget {
    Q_OBJECT
public:
    explicit SearchPanel(SnippetManager *mgr, QWidget *parent = nullptr);

    void refreshCategoryTree();
    void refreshSearch();
    void refreshThumbnailList();
    QString currentCategory() const;

signals:
    void snippetSelected(const QString &id);
    void searchQueryChanged(const QString &query);

private:
    void setupUI();
    void buildCategoryTree(QStandardItem *parent, const QString &path,
                           const QMap<QString, int> &counts, int depth = 0);
    void showCategoryContextMenu(const QPoint &pos);
    void renameCategoryItem(QStandardItem *item);
    void deleteCategoryItem(QStandardItem *item);
    QIcon loadThumbnailIcon(const QString &snippetId) const;

    SnippetManager *snippetMgr;

    QLineEdit *searchBox;
    QTreeView *categoryTree;
    QListView *thumbnailList;
    QStandardItemModel *categoryModel;
    QStandardItemModel *thumbnailModel;
    QMenu *categoryCtxMenu;
    QTimer *searchDebounceTimer;
};

