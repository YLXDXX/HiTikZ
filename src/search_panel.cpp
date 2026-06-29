#include "search_panel.h"
#include "snippet_properties_dialog.h"
#include "snippet_manager.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QRegularExpression>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QMimeData>
#include <QDataStream>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QDir>
#include <QApplication>

SearchPanel::SearchPanel(SnippetManager *mgr, QWidget *parent)
    : QWidget(parent), snippetMgr(mgr)
{
    setupUI();

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
            if (QApplication::keyboardModifiers() & Qt::ControlModifier) return;
            QString id = current.data(Qt::UserRole).toString();
            if (!id.isEmpty())
                emit snippetSelected(id);
        });

    connect(categoryTree->selectionModel(), &QItemSelectionModel::currentChanged,
        this, [this](const QModelIndex &current, const QModelIndex &) {
            if (!current.isValid()) return;
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
    layout->addWidget(categoryTree, 1);
    layout->addWidget(thumbnailList, 2);
}

void SearchPanel::refreshSearch()
{
    QString query = searchBox->text().trimmed();
    thumbnailModel->clear();

    QList<SearchResult> results = snippetMgr->searchSnippets(query);
    for (const SearchResult &r : results) {
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
        } else if (!category.isEmpty() && !r.snippet.category.startsWith(category)) {
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

void SearchPanel::showCategoryContextMenu(const QPoint &pos)
{
    QModelIndex idx = categoryTree->indexAt(pos);
    if (!idx.isValid()) return;
    QStandardItem *item = categoryModel->itemFromIndex(idx);
    if (!item) return;

    QString catData = item->data(Qt::UserRole).toString();
    bool isAll = catData.isEmpty();

    QList<QAction*> actions = categoryCtxMenu->actions();
    for (QAction *act : actions) {
        if (act->text() == QStringLiteral("新建大类"))
            act->setVisible(isAll);
        else if (act->text() == QStringLiteral("重命名分类")
              || act->text() == QStringLiteral("删除分类")
              || act->text() == QStringLiteral("新建子分类"))
            act->setVisible(!isAll);
    }

    categoryCtxMenu->popup(categoryTree->viewport()->mapToGlobal(pos));
}

void SearchPanel::renameCategoryItem(QStandardItem *item)
{
    if (!item) return;
    QString oldCat = item->data(Qt::UserRole).toString();
    if (oldCat.isEmpty()) return;

    bool ok;
    QString newName = QInputDialog::getText(this, QStringLiteral("重命名分类"),
        QStringLiteral("新名称:"), QLineEdit::Normal, oldCat, &ok);
    if (!ok || newName.isEmpty() || newName == oldCat) return;

    int count = snippetMgr->renameCategory(oldCat, newName);
    Q_UNUSED(count);
    refreshCategoryTree();
    refreshSearch();
}

void SearchPanel::deleteCategoryItem(QStandardItem *item)
{
    if (!item) return;
    QString cat = item->data(Qt::UserRole).toString();
    if (cat.isEmpty()) return;

    int ret = QMessageBox::warning(this, QStringLiteral("删除分类"),
        QStringLiteral("确定删除分类 \"%1\" 及其所有片段吗？").arg(cat),
        QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    int count = snippetMgr->deleteCategory(cat);
    Q_UNUSED(count);
    refreshCategoryTree();
    refreshSearch();
}

void SearchPanel::onThumbnailRightClick(const QPoint &pos)
{
    QModelIndex idx = thumbnailList->indexAt(pos);
    if (!idx.isValid()) return;
    QString id = idx.data(Qt::UserRole).toString();
    if (id.isEmpty()) return;

    SnippetPropertiesDialog dlg(id, snippetMgr, this);
    if (dlg.exec() == QDialog::Accepted) {
        refreshCategoryTree();
        refreshSearch();
    }
}

void SearchPanel::showThumbnailContextMenu(const QPoint &pos)
{
    QModelIndex idx = thumbnailList->indexAt(pos);
    QStringList selectedIds = getSelectedSnippetIds();

    if (selectedIds.size() > 1) {
        QList<QAction*> actions = thumbnailCtxMenu->actions();
        for (QAction *act : actions) {
            act->setVisible(true);
        }
        thumbnailCtxMenu->popup(thumbnailList->viewport()->mapToGlobal(pos));
    } else if (idx.isValid()) {
        QString id = idx.data(Qt::UserRole).toString();
        if (!id.isEmpty()) {
            SnippetPropertiesDialog dlg(id, snippetMgr, this);
            if (dlg.exec() == QDialog::Accepted) {
                refreshCategoryTree();
                refreshSearch();
            }
        }
    }
}

QStringList SearchPanel::getSelectedSnippetIds() const
{
    QStringList ids;
    QModelIndexList selected = thumbnailList->selectionModel()->selectedIndexes();
    QSet<QString> seen;
    for (const QModelIndex &idx : selected) {
        QString id = idx.data(Qt::UserRole).toString();
        if (!id.isEmpty() && !seen.contains(id)) {
            ids.append(id);
            seen.insert(id);
        }
    }
    return ids;
}

QIcon SearchPanel::loadThumbnailIcon(const QString &snippetId) const
{
    QString basePath = snippetMgr->isPresetId(snippetId)
        ? snippetMgr->getPresetPath() : snippetMgr->getBasePath();
    QString pngPath = basePath + snippetId + "/preview.png";
    if (QFile::exists(pngPath))
        return QIcon(pngPath);
    return QIcon();
}

bool SearchPanel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == categoryTree->viewport() && event->type() == QEvent::Drop) {
        QDropEvent *de = static_cast<QDropEvent *>(event);
        const QMimeData *mime = de->mimeData();
        QModelIndex targetIdx = categoryTree->indexAt(de->position().toPoint());
        QString targetCat;
        if (targetIdx.isValid())
            targetCat = targetIdx.data(Qt::UserRole).toString();

        if (mime->hasFormat("application/x-qabstractitemmodeldatalist")) {
            QByteArray data = mime->data("application/x-qabstractitemmodeldatalist");
            QDataStream stream(&data, QIODevice::ReadOnly);
            QSet<QString> seen;
            bool anyUpdated = false;

            while (!stream.atEnd()) {
                int row, col;
                QMap<int, QVariant> roleData;
                stream >> row >> col >> roleData;
                QString snippetId = roleData[Qt::UserRole].toString();

                if (!snippetId.isEmpty() && !seen.contains(snippetId)) {
                    seen.insert(snippetId);
                    snippetMgr->updateSnippetCategory(snippetId, targetCat);
                    anyUpdated = true;
                }
            }

            if (anyUpdated) {
                refreshCategoryTree();
                refreshSearch();
                de->accept();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
