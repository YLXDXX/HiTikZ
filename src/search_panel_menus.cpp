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
                // Save the current category before refreshing the tree,
                // so we can feed it into refreshSearch directly.
                QString savedCategory;
                QModelIndex savedCatIdx = categoryTree->currentIndex();
                if (savedCatIdx.isValid())
                    savedCategory = savedCatIdx.data(Qt::UserRole).toString();

                // Block signals during tree rebuild to avoid spurious
                // category-change events that would reset the thumbnail list.
                {
                    const QSignalBlocker thumbBlocker(thumbnailList->selectionModel());
                    const QSignalBlocker catBlocker(categoryTree->selectionModel());
                    refreshCategoryTree();
                }

                // Defer tree selection restore — expandAll() triggers internal
                // layout updates that can overwrite a synchronous setCurrentIndex.
                //
                // NOTE: findItems(text, flags, column) treats its 3rd parameter
                // as column number, not role. Passing Qt::UserRole (256) means
                // searching column 256 of a 1-column model, always returning
                // empty. Use match() instead to search by Qt::UserRole.
                QTimer::singleShot(0, this, [this, savedCategory]() {
                    m_suppressSelectEmit = true;
                    QModelIndexList found = categoryModel->match(
                        categoryModel->index(0, 0),
                        Qt::UserRole,
                        savedCategory,
                        1,
                        Qt::MatchExactly | Qt::MatchRecursive);
                    if (!found.isEmpty()) {
                        categoryTree->setCurrentIndex(found.first());
                        categoryTree->scrollTo(found.first());
                    }
                    m_suppressSelectEmit = false;
                });

                // Supply the category directly to bypass any tree selection timing issues
                m_pendingCatFilter = savedCategory;
                m_hasPendingCatFilter = true;
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
    if (obj == thumbnailList->viewport() && event->type() == QEvent::Drop) {
        QDropEvent *de = static_cast<QDropEvent *>(event);
        const QMimeData *mime = de->mimeData();
        if (!mime->hasFormat("application/x-qabstractitemmodeldatalist"))
            return false;

        QByteArray data = mime->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&data, QIODevice::ReadOnly);
        QList<QString> draggedIds;
        QList<QStandardItem *> draggedItems;
        while (!stream.atEnd() && stream.status() == QDataStream::Ok) {
            int row, col;
            QMap<int, QVariant> roleData;
            stream >> row >> col >> roleData;
            QString id = roleData[Qt::UserRole].toString();
            if (id.isEmpty()) continue;
            QStandardItem *item = nullptr;
            for (int i = 0; i < thumbnailModel->rowCount(); ++i) {
                if (thumbnailModel->index(i, 0).data(Qt::UserRole).toString() == id) {
                    item = thumbnailModel->item(i);
                    break;
                }
            }
            if (item && !draggedIds.contains(id)) {
                draggedIds.append(id);
                draggedItems.append(item->clone());
                thumbnailModel->removeRow(item->row());
            }
        }

        if (draggedItems.isEmpty()) return false;

        QPoint dropPt = de->position().toPoint();
        QModelIndex dropIdx = thumbnailList->indexAt(dropPt);
        int insertRow = dropIdx.isValid() ? dropIdx.row() : thumbnailModel->rowCount();

        for (int i = 0; i < draggedItems.size(); ++i) {
            thumbnailModel->insertRow(insertRow + i, draggedItems[i]);
        }

        QStringList orderedIds;
        for (int i = 0; i < thumbnailModel->rowCount(); ++i)
            orderedIds.append(thumbnailModel->index(i, 0).data(Qt::UserRole).toString());
        if (orderedIds.size() > 1)
            snippetMgr->reorderSnippets(orderedIds);

        de->accept();
        return true;
    }

    if (obj == categoryTree->viewport() && event->type() == QEvent::Drop) {
        QDropEvent *de = static_cast<QDropEvent *>(event);
        const QMimeData *mime = de->mimeData();
        QModelIndex targetIdx = categoryTree->indexAt(de->position().toPoint());

        if (mime->hasFormat("application/x-qabstractitemmodeldatalist")) {
            QByteArray data = mime->data("application/x-qabstractitemmodeldatalist");
            QDataStream stream(&data, QIODevice::ReadOnly);
            QSet<QString> seen;
            bool isCategoryDrag = false;
            QStringList draggedIds;

            while (!stream.atEnd() && stream.status() == QDataStream::Ok) {
                int row, col;
                QMap<int, QVariant> roleData;
                stream >> row >> col >> roleData;
                QString itemId = roleData[Qt::UserRole].toString();
                if (itemId.isEmpty()) continue;
                if (seen.contains(itemId)) continue;
                seen.insert(itemId);
                draggedIds.append(itemId);

                if (itemId != QLatin1String("__uncategorized__")
                    && !itemId.isEmpty() && !itemId.contains('-')) {
                    isCategoryDrag = true;
                }
            }

            if (isCategoryDrag) {
                QStringList savedOrder;
                QStandardItem *root = categoryModel->invisibleRootItem();
                for (int i = 0; i < root->rowCount(); ++i) {
                    collectOrderedCategories(root->child(i), savedOrder);
                }
                if (!savedOrder.isEmpty()) {
                    snippetMgr->saveCategoryOrder(savedOrder);
                    refreshCategoryTree();
                    de->accept();
                    return true;
                }
            } else {
                QString targetCat;
                if (targetIdx.isValid())
                    targetCat = targetIdx.data(Qt::UserRole).toString();

                bool anyMoved = false;
                for (const QString &snippetId : draggedIds) {
                    snippetMgr->updateSnippetCategory(snippetId, targetCat);
                    anyMoved = true;
                }

                if (anyMoved) {
                    refreshCategoryTree();
                    refreshSearch();
                    de->accept();
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void SearchPanel::collectOrderedCategories(QStandardItem *item, QStringList &order)
{
    if (!item) return;
    QString cat = item->data(Qt::UserRole).toString();
    if (cat != QLatin1String("__uncategorized__") && !cat.isEmpty())
        order.append(cat);
    for (int i = 0; i < item->rowCount(); ++i)
        collectOrderedCategories(item->child(i), order);
}
