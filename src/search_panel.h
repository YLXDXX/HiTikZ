#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QTreeView>
#include <QListView>
#include <QStandardItemModel>
#include <QMenu>
#include <QTimer>
#include <QStringList>
#include <QSet>
#include <QPushButton>

class SnippetManager;

class SearchPanel : public QWidget {
    Q_OBJECT
public:
    explicit SearchPanel(SnippetManager *mgr, QWidget *parent = nullptr);

    void refreshCategoryTree();
    void refreshSearch();
    void refreshThumbnailList();
    void refreshTagFilter();
    void applyUIFont(const QFont &font);
    QString currentCategory() const;

signals:
    void snippetSelected(const QString &id);
    void searchQueryChanged(const QString &query);
    void batchExportRequested(const QStringList &ids);
    void batchCategoryChangeRequested(const QStringList &ids);
    void batchDeleteRequested(const QStringList &ids);
    void exportAllRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void buildCategoryTree(QStandardItem *parent, const QString &path,
                           const QMap<QString, int> &counts, int depth = 0);
    void applyTagRowCollapse();
    void showCategoryContextMenu(const QPoint &pos);
    void renameCategoryItem(QStandardItem *item);
    void deleteCategoryItem(QStandardItem *item);
    void showThumbnailContextMenu(const QPoint &pos);
    QStringList getSelectedSnippetIds() const;
    QIcon loadThumbnailIcon(const QString &snippetId) const;

    SnippetManager *snippetMgr;

    QLineEdit *searchBox;
    QTreeView *categoryTree;
    QListView *thumbnailList;
    QStandardItemModel *categoryModel;
    QStandardItemModel *thumbnailModel;
    QMenu *categoryCtxMenu;
    QMenu *thumbnailCtxMenu;
    QTimer *searchDebounceTimer;
    QWidget *tagFilterWidget;
    QSet<QString> m_selectedTags;
    QWidget *m_tagButtonContainer = nullptr;
    class FlowLayout *m_tagFlowLayout = nullptr;
    QPushButton *m_moreTagsBtn = nullptr;
    QStringList m_allTagNames;
    bool m_suppressSelectEmit = false;
    QTimer *m_tagCollapseTimer = nullptr;
    bool m_inTagCollapse = false;
    static const int kMaxTagRows = 2;
};
