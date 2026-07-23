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

    // ── Test/introspection accessors for the tag filter ──
    // The set of tag names that are currently active as a filter.
    QSet<QString> selectedTags() const { return m_selectedTags; }
    // All distinct tag names currently offered in the tag strip (rebuilt by
    // refreshTagFilter from every snippet's metadata).
    QStringList allTagNames() const { return m_allTagNames; }
    // Programmatically toggle a tag's selection (mirrors clicking its button),
    // then refresh the search. Used by tests and callers that need to drive the
    // filter without a live button.
    void setTagSelected(const QString &tag, bool selected);
    // Number of visible thumbnails currently shown.
    int thumbnailCount() const;
    // Opaque identity of the tag-button strip container. Stays the same across
    // a refreshTagFilter() that finds an unchanged tag set (no rebuild → no
    // flicker); changes when the strip is actually rebuilt. For tests only.
    const void *tagStripToken() const { return m_tagButtonContainer; }

signals:
    void snippetSelected(const QString &id);
    void searchQueryChanged(const QString &query);
    void batchExportRequested(const QStringList &ids);
    void batchCategoryChangeRequested(const QStringList &ids);
    void batchDeleteRequested(const QStringList &ids);
    void exportAllRequested();
    void copySnippetRequested(const QString &id);
    void addSnippetRequested(const QString &category);

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
    static void collectOrderedCategories(QStandardItem *item, QStringList &order);

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
    QString m_pendingCatFilter;
    bool m_hasPendingCatFilter = false;
    static const int kMaxTagRows = 2;
};
