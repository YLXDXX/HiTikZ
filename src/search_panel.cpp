#include "search_panel.h"
#include "snippet_properties_dialog.h"
#include "snippet_manager.h"
#include "flow_layout.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QRegularExpression>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QMimeData>
#include <QDataStream>
#include <QDropEvent>
#include <QEvent>
#include <QResizeEvent>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QPushButton>
#include <QScrollArea>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QScrollBar>
#include <QSplitter>
#include <QSignalBlocker>

SearchPanel::SearchPanel(SnippetManager *mgr, QWidget *parent)
    : QWidget(parent), snippetMgr(mgr)
{
    setupUI();

    m_tagCollapseTimer = new QTimer(this);
    m_tagCollapseTimer->setSingleShot(true);
    m_tagCollapseTimer->setInterval(150);
    connect(m_tagCollapseTimer, &QTimer::timeout, this, &SearchPanel::applyTagRowCollapse);

    searchDebounceTimer = new QTimer(this);
    searchDebounceTimer->setSingleShot(true);
    searchDebounceTimer->setInterval(150);
    connect(searchDebounceTimer, &QTimer::timeout, this, &SearchPanel::refreshSearch);

    connect(searchBox, &QLineEdit::textChanged, this, [this](const QString &) {
        searchDebounceTimer->start();
    });

    connect(thumbnailList->selectionModel(), &QItemSelectionModel::currentChanged,
        this, [this](const QModelIndex &current, const QModelIndex &) {
            if (!current.isValid()) return;
            if (m_suppressSelectEmit) return;
            if (QApplication::keyboardModifiers() & Qt::ControlModifier) return;
            QString id = current.data(Qt::UserRole).toString();
            if (!id.isEmpty())
                emit snippetSelected(id);
        });

    connect(thumbnailList, &QListView::clicked,
        this, [this](const QModelIndex &index) {
            if (!index.isValid()) return;
            if (m_suppressSelectEmit) return;
            if (QApplication::keyboardModifiers() & Qt::ControlModifier) return;
            QString id = index.data(Qt::UserRole).toString();
            if (!id.isEmpty())
                emit snippetSelected(id);
        });

    connect(categoryTree->selectionModel(), &QItemSelectionModel::currentChanged,
        this, [this](const QModelIndex &current, const QModelIndex &) {
            if (!current.isValid()) return;
            if (m_suppressSelectEmit) return;
            refreshThumbnailList();
        });

    connect(categoryTree, &QTreeView::customContextMenuRequested,
        this, &SearchPanel::showCategoryContextMenu);
}

void SearchPanel::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    searchBox = new QLineEdit;
    searchBox->setPlaceholderText(QStringLiteral("搜索名称或简介..."));
    searchBox->setClearButtonEnabled(true);

    categoryTree = new QTreeView;
    categoryTree->setHeaderHidden(true);
    categoryTree->setRootIsDecorated(true);
    categoryTree->setContextMenuPolicy(Qt::CustomContextMenu);
    categoryTree->setAcceptDrops(true);
    categoryTree->setDropIndicatorShown(true);
    categoryTree->setDragDropMode(QAbstractItemView::DropOnly);
    categoryTree->viewport()->setAcceptDrops(true);
    categoryModel = new QStandardItemModel(this);
    categoryTree->setModel(categoryModel);
    categoryTree->viewport()->installEventFilter(this);

    thumbnailList = new QListView;
    thumbnailList->setViewMode(QListView::IconMode);
    thumbnailList->setIconSize(QSize(96, 96));
    thumbnailList->setGridSize(QSize(120, 130));
    thumbnailList->setResizeMode(QListView::Adjust);
    thumbnailList->setWordWrap(true);
    thumbnailList->setDragEnabled(true);
    thumbnailList->setDragDropMode(QAbstractItemView::DragOnly);
    thumbnailList->setContextMenuPolicy(Qt::CustomContextMenu);
    thumbnailList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    thumbnailModel = new QStandardItemModel(this);
    thumbnailList->setModel(thumbnailModel);

    connect(thumbnailList, &QListView::customContextMenuRequested,
        this, &SearchPanel::showThumbnailContextMenu);

    categoryCtxMenu = new QMenu(this);
    QAction *renameCatAct = categoryCtxMenu->addAction(QStringLiteral("重命名分类"));
    QAction *deleteCatAct = categoryCtxMenu->addAction(QStringLiteral("删除分类"));
    QAction *newSubCatAct = categoryCtxMenu->addAction(QStringLiteral("新建子分类"));
    QAction *newTopCatAct = categoryCtxMenu->addAction(QStringLiteral("新建大类"));
    newTopCatAct->setVisible(false);

    auto getEffectiveCatItem = [this]() -> QStandardItem* {
        QModelIndex idx = categoryTree->currentIndex();
        return idx.isValid() ? categoryModel->itemFromIndex(idx) : nullptr;
    };

    connect(renameCatAct, &QAction::triggered, this, [this, getEffectiveCatItem]() {
        renameCategoryItem(getEffectiveCatItem());
    });
    connect(deleteCatAct, &QAction::triggered, this, [this, getEffectiveCatItem]() {
        deleteCategoryItem(getEffectiveCatItem());
    });
    connect(newSubCatAct, &QAction::triggered, this, [this, getEffectiveCatItem]() {
        QStandardItem *item = getEffectiveCatItem();
        if (!item) return;
        QString parentCat = item->data(Qt::UserRole).toString();
        bool ok;
        QString name = QInputDialog::getText(this, QStringLiteral("新建子分类"),
            QStringLiteral("子分类名称:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            QString newCat = parentCat.isEmpty() ? name : parentCat + "/" + name;
            snippetMgr->createSnippet(QStringLiteral("新片段"), newCat);
            refreshCategoryTree();
            refreshSearch();
        }
    });
    connect(newTopCatAct, &QAction::triggered, this, [this]() {
        bool ok;
        QString name = QInputDialog::getText(this, QStringLiteral("新建大类"),
            QStringLiteral("大类名称:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            snippetMgr->createSnippet(QStringLiteral("新片段"), name);
            refreshCategoryTree();
            refreshSearch();
        }
    });

    thumbnailCtxMenu = new QMenu(this);
    QAction *batchExportAct = thumbnailCtxMenu->addAction(QStringLiteral("批量导出所选"));
    QAction *batchCategoryAct = thumbnailCtxMenu->addAction(QStringLiteral("修改分类"));
    QAction *batchDeleteAct = thumbnailCtxMenu->addAction(QStringLiteral("删除所选"));
    thumbnailCtxMenu->addSeparator();
    QAction *selectAllAct = thumbnailCtxMenu->addAction(QStringLiteral("全选"));
    QAction *exportAllAct = thumbnailCtxMenu->addAction(QStringLiteral("导出所有"));

    connect(batchExportAct, &QAction::triggered, this, [this]() {
        QStringList ids = getSelectedSnippetIds();
        if (!ids.isEmpty()) emit batchExportRequested(ids);
    });
    connect(batchCategoryAct, &QAction::triggered, this, [this]() {
        QStringList ids = getSelectedSnippetIds();
        if (!ids.isEmpty()) emit batchCategoryChangeRequested(ids);
    });
    connect(batchDeleteAct, &QAction::triggered, this, [this]() {
        QStringList ids = getSelectedSnippetIds();
        if (!ids.isEmpty()) emit batchDeleteRequested(ids);
    });
    connect(selectAllAct, &QAction::triggered, this, [this]() {
        thumbnailList->selectAll();
    });
    connect(exportAllAct, &QAction::triggered, this, [this]() {
        emit exportAllRequested();
    });

    layout->addWidget(searchBox);

    tagFilterWidget = new QWidget;
    QVBoxLayout *tagFilterLayout = new QVBoxLayout(tagFilterWidget);
    tagFilterLayout->setContentsMargins(0, 2, 0, 2);
    layout->addWidget(tagFilterWidget);

    QSplitter *treeThumbSplitter = new QSplitter(Qt::Vertical);
    treeThumbSplitter->setHandleWidth(5);
    treeThumbSplitter->setStyleSheet(
        QStringLiteral("QSplitter::handle { background: #c0c0c0; }"
                       "QSplitter::handle:hover { background: #4a90d9; }"));
    treeThumbSplitter->addWidget(categoryTree);
    treeThumbSplitter->addWidget(thumbnailList);
    treeThumbSplitter->setStretchFactor(0, 1);
    treeThumbSplitter->setStretchFactor(1, 2);
    treeThumbSplitter->setChildrenCollapsible(false);
    layout->addWidget(treeThumbSplitter, 1);
}

void SearchPanel::applyUIFont(const QFont &font)
{
    categoryTree->setFont(font);
    thumbnailList->setFont(font);
}

void SearchPanel::refreshSearch()
{
    QString query = searchBox->text().trimmed();

    // Remember currently selected snippet to restore after rebuild
    QString prevSelectedId;
    QModelIndex prevIdx = thumbnailList->selectionModel()
        ? thumbnailList->selectionModel()->currentIndex() : QModelIndex();
    if (prevIdx.isValid())
        prevSelectedId = prevIdx.data(Qt::UserRole).toString();

    // Remember scroll position
    int scrollValue = thumbnailList->verticalScrollBar()
        ? thumbnailList->verticalScrollBar()->value() : 0;

    // Category filter: use explicit override if set, otherwise read from tree
    QString currentCat;
    if (m_hasPendingCatFilter) {
        currentCat = m_pendingCatFilter;
        m_hasPendingCatFilter = false;
        m_pendingCatFilter.clear();
    } else {
        QModelIndex catIdx = categoryTree->currentIndex();
        if (catIdx.isValid())
            currentCat = catIdx.data(Qt::UserRole).toString();
    }

    thumbnailModel->clear();

    QList<SearchResult> results = snippetMgr->searchSnippets(query);
    for (const SearchResult &r : results) {
        if (!m_selectedTags.isEmpty()) {
            bool hasAllTags = true;
            for (const QString &tag : m_selectedTags) {
                if (!r.snippet.tags.contains(tag)) {
                    hasAllTags = false;
                    break;
                }
            }
            if (!hasAllTags) continue;
        }

        if (currentCat == "__uncategorized__") {
            if (!r.snippet.category.isEmpty()) continue;
        } else if (!currentCat.isEmpty()
                   && !SnippetManager::categoryMatches(r.snippet.category, currentCat)) {
            continue;
        }

        QString label = r.snippet.isPreset ? QStringLiteral("[预设] ") + r.snippet.name : r.snippet.name;
        QStandardItem *item = new QStandardItem(label);
        item->setData(r.snippet.id, Qt::UserRole);
        item->setToolTip(QString("%1%2 (分数: %3)\n%4")
            .arg(r.snippet.isPreset ? QStringLiteral("[预设] ") : QString())
            .arg(r.snippet.name).arg(r.score).arg(r.snippet.description));

        QIcon icon = loadThumbnailIcon(r.snippet.id);
        if (!icon.isNull())
            item->setIcon(icon);

        thumbnailModel->appendRow(item);
    }

    // Restore scroll position and selection — deferred to next event loop
    // iteration so the view has processed the model reset and laid out items.
    QTimer::singleShot(0, this, [this, scrollValue, prevSelectedId]() {
        if (thumbnailList->verticalScrollBar())
            thumbnailList->verticalScrollBar()->setValue(scrollValue);

        if (!prevSelectedId.isEmpty()) {
            m_suppressSelectEmit = true;
            for (int i = 0; i < thumbnailModel->rowCount(); ++i) {
                QModelIndex idx = thumbnailModel->index(i, 0);
                if (idx.data(Qt::UserRole).toString() == prevSelectedId) {
                    thumbnailList->selectionModel()->setCurrentIndex(
                        idx, QItemSelectionModel::ClearAndSelect);
                    break;
                }
            }
            m_suppressSelectEmit = false;
        }
    });
}

void SearchPanel::refreshThumbnailList()
{
    QModelIndex current = categoryTree->currentIndex();
    if (!current.isValid()) return;
    QString category = current.data(Qt::UserRole).toString();
    thumbnailModel->clear();

    QList<SearchResult> results = snippetMgr->searchSnippets("");
    for (const SearchResult &r : results) {
        if (category == "__uncategorized__") {
            if (!r.snippet.category.isEmpty()) continue;
        } else if (!category.isEmpty()
                   && !SnippetManager::categoryMatches(r.snippet.category, category)) {
            continue;
        }
        QString label = r.snippet.isPreset ? QStringLiteral("[预设] ") + r.snippet.name : r.snippet.name;
        QStandardItem *item = new QStandardItem(label);
        item->setData(r.snippet.id, Qt::UserRole);
        item->setToolTip(r.snippet.description);

        QIcon icon = loadThumbnailIcon(r.snippet.id);
        if (!icon.isNull())
            item->setIcon(icon);

        thumbnailModel->appendRow(item);
    }
}

QString SearchPanel::currentCategory() const
{
    QModelIndex idx = categoryTree->currentIndex();
    return idx.isValid() ? idx.data(Qt::UserRole).toString() : QString();
}

void SearchPanel::refreshCategoryTree()
{
    categoryModel->clear();
    QStandardItem *rootItem = categoryModel->invisibleRootItem();

    QMap<QString, int> catCounts = snippetMgr->getCategoryCounts(true);

    int totalCount = 0;
    for (auto it = catCounts.constBegin(); it != catCounts.constEnd(); ++it)
        totalCount += it.value();

    int uncategorizedCount = snippetMgr->getUncategorizedCount(true);
    totalCount += uncategorizedCount;

    QStandardItem *allItem = new QStandardItem(QStringLiteral("全部 (%1)").arg(totalCount));
    allItem->setData("", Qt::UserRole);
    allItem->setEditable(false);
    rootItem->appendRow(allItem);

    if (uncategorizedCount > 0) {
        QStandardItem *uncatItem = new QStandardItem(QStringLiteral("未分类 (%1)").arg(uncategorizedCount));
        uncatItem->setData("__uncategorized__", Qt::UserRole);
        uncatItem->setEditable(false);
        rootItem->appendRow(uncatItem);
    }

    QStringList categories = snippetMgr->getAllCategories();
    for (const QString &cat : categories) {
        buildCategoryTree(rootItem, cat, catCounts);
    }

    categoryTree->expandAll();
}

void SearchPanel::buildCategoryTree(QStandardItem *parent, const QString &path,
                                     const QMap<QString, int> &counts, int depth)
{
    Q_UNUSED(depth);
    if (path.isEmpty()) return;

    int slashPos = path.indexOf('/');
    QString name = (slashPos >= 0) ? path.left(slashPos) : path;
    QString remaining = (slashPos >= 0) ? path.mid(slashPos + 1) : QString();
    QString fullPath = parent->data(Qt::UserRole).toString();
    if (!fullPath.isEmpty()) fullPath += "/";
    fullPath += name;

    int subtreeCount = 0;
    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
        if (it.key() == fullPath || it.key().startsWith(fullPath + "/"))
            subtreeCount += it.value();
    }

    QStandardItem *child = nullptr;
    for (int i = 0; i < parent->rowCount(); ++i) {
        QStandardItem *sibling = parent->child(i);
        if (sibling->data(Qt::UserRole).toString() == fullPath) {
            child = sibling;
            break;
        }
    }

    QString label = subtreeCount > 0
        ? QString("%1 (%2)").arg(name).arg(subtreeCount)
        : name;

    if (!child) {
        child = new QStandardItem(label);
        child->setData(fullPath, Qt::UserRole);
        child->setEditable(false);
        parent->appendRow(child);
    } else {
        child->setText(label);
    }

    if (!remaining.isEmpty()) {
        buildCategoryTree(child, remaining, counts, depth + 1);
    }
}

void SearchPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_tagCollapseTimer && !m_inTagCollapse)
        m_tagCollapseTimer->start();
}
