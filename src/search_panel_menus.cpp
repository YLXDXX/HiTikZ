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
#include <QDragMoveEvent>
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

    if (selectedIds.isEmpty()) return;

    if (!thumbnailList->selectionModel()->isSelected(idx) && idx.isValid()) {
        thumbnailList->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
        selectedIds = getSelectedSnippetIds();
    }

    QMenu menu(this);

    QAction *openAct = nullptr;
    QAction *propAct = nullptr;

    if (selectedIds.size() == 1) {
        openAct = menu.addAction(QStringLiteral("打开"));
        menu.addSeparator();
    }

    QAction *copyAct = menu.addAction(QStringLiteral("复制"));
    QAction *delAct = menu.addAction(QStringLiteral("删除"));

    if (selectedIds.size() == 1) {
        menu.addSeparator();
        propAct = menu.addAction(QStringLiteral("属性"));
    }

    QAction *chosen = menu.exec(thumbnailList->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == openAct) {
        emit snippetSelected(selectedIds.first());
    } else if (chosen == copyAct) {
        for (const QString &id : selectedIds)
            emit copySnippetRequested(id);
    } else if (chosen == delAct) {
        emit batchDeleteRequested(selectedIds);
    } else if (chosen == propAct) {
        QString id = selectedIds.first();
        SnippetPropertiesDialog dlg(id, snippetMgr, this);
        if (dlg.exec() == QDialog::Accepted) {
            QString savedCategory;
            QModelIndex savedCatIdx = categoryTree->currentIndex();
            if (savedCatIdx.isValid()) {
                savedCategory = savedCatIdx.data(Qt::UserRole).toString();
            }
            {
                const QSignalBlocker thumbBlocker(thumbnailList->selectionModel());
                const QSignalBlocker catBlocker(categoryTree->selectionModel());
                refreshCategoryTree();
            }
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
            m_pendingCatFilter = savedCategory;
            m_hasPendingCatFilter = true;
            refreshSearch();
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
        QSet<QString> draggedIdSet;

        while (!stream.atEnd() && stream.status() == QDataStream::Ok) {
            int row, col;
            QMap<int, QVariant> roleData;
            stream >> row >> col >> roleData;
            QString id = roleData[Qt::UserRole].toString();
            if (!id.isEmpty())
                draggedIdSet.insert(id);
        }

        if (draggedIdSet.isEmpty()) return false;

        QStringList draggedIds;
        QStringList restIds;
        for (int i = 0; i < thumbnailModel->rowCount(); ++i) {
            QString id = thumbnailModel->index(i, 0).data(Qt::UserRole).toString();
            if (draggedIdSet.contains(id))
                draggedIds.append(id);
            else
                restIds.append(id);
        }

        QPoint dropPt = de->position().toPoint();
        QModelIndex dropIdx = thumbnailList->indexAt(dropPt);
        int insertPos = restIds.size();

        if (dropIdx.isValid()) {
            QString targetId = dropIdx.data(Qt::UserRole).toString();
            int idx = restIds.indexOf(targetId);
            if (idx >= 0) {
                QRect itemRect = thumbnailList->visualRect(dropIdx);
                bool dropAbove = !itemRect.isNull()
                    && dropPt.y() < itemRect.center().y();
                insertPos = dropAbove ? idx : idx + 1;
            }
        }

        for (int i = 0; i < draggedIds.size(); ++i)
            restIds.insert(insertPos + i, draggedIds[i]);

        de->accept();
        QStringList finalOrder = restIds;
        QTimer::singleShot(0, this, [this, finalOrder]() {
            thumbnailModel->clear();
            for (const QString &id : finalOrder) {
                Snippet s = snippetMgr->loadSnippet(id);
                QString label = s.isPreset
                    ? QStringLiteral("[预设] ") + s.name : s.name;
                QStandardItem *item = new QStandardItem(label);
                item->setData(id, Qt::UserRole);
                item->setToolTip(s.description);
                QIcon icon = loadThumbnailIcon(id);
                if (!icon.isNull()) item->setIcon(icon);
                thumbnailModel->appendRow(item);
            }
            if (finalOrder.size() > 1)
                snippetMgr->reorderSnippets(finalOrder);
        });
        return true;
    }

    if (obj == categoryTree->viewport() && event->type() == QEvent::Drop) {
        QDropEvent *de = static_cast<QDropEvent *>(event);
        const QMimeData *mime = de->mimeData();
        QModelIndex targetIdx = categoryTree->indexAt(de->position().toPoint());

        bool fromThumbnails = (de->source() == thumbnailList->viewport()
                               || de->source() == thumbnailList);

        if (!mime->hasFormat("application/x-qabstractitemmodeldatalist"))
            return false;

        QByteArray data = mime->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&data, QIODevice::ReadOnly);
        QStringList draggedIds;

        while (!stream.atEnd() && stream.status() == QDataStream::Ok) {
            int row, col;
            QMap<int, QVariant> roleData;
            stream >> row >> col >> roleData;
            QString itemId = roleData[Qt::UserRole].toString();
            if (itemId.isEmpty()) continue;
            if (itemId == QLatin1String("__uncategorized__")) continue;
            if (!draggedIds.contains(itemId))
                draggedIds.append(itemId);
        }

        if (draggedIds.isEmpty()) return false;

        if (fromThumbnails) {
            QString targetCat;
            if (targetIdx.isValid()) {
                targetCat = targetIdx.data(Qt::UserRole).toString();
                if (targetCat == QLatin1String("__uncategorized__"))
                    targetCat.clear();
                if (targetCat.isEmpty() && targetIdx.isValid()) {
                    de->accept();
                    return true;
                }
            }
            for (const QString &snippetId : draggedIds)
                snippetMgr->updateSnippetCategory(snippetId, targetCat);
            refreshCategoryTree();
            refreshSearch();
            de->accept();
            return true;
        }

        QString targetCat;
        bool dropBelow = false;

        if (targetIdx.isValid()) {
            targetCat = targetIdx.data(Qt::UserRole).toString();
            if (targetCat.isEmpty()
                || targetCat == QLatin1String("__uncategorized__")) {
                targetCat.clear();
            } else {
                QRect itemRect = categoryTree->visualRect(targetIdx);
                if (!itemRect.isNull() && itemRect.height() > 0) {
                    dropBelow = de->position().toPoint().y()
                        >= itemRect.center().y();
                }
            }
        }

        for (const QString &draggedId : draggedIds) {
            if (draggedId == targetCat) {
                de->ignore();
                return true;
            }
            if (!targetCat.isEmpty()
                && targetCat.startsWith(draggedId + QLatin1Char('/'))) {
                de->ignore();
                return true;
            }
        }

        QStringList savedOrder = snippetMgr->loadCategoryOrder();
        if (savedOrder.isEmpty())
            savedOrder = snippetMgr->getAllCategories();

        QStringList toMove;
        for (const QString &draggedId : draggedIds) {
            toMove.append(draggedId);
            for (const QString &cat : savedOrder) {
                if (cat.startsWith(draggedId + QLatin1Char('/')))
                    toMove.append(cat);
            }
        }

        QStringList toMoveUnique;
        QSet<QString> seen;
        for (const QString &cat : toMove) {
            if (!seen.contains(cat)) {
                seen.insert(cat);
                toMoveUnique.append(cat);
            }
        }

        QStringList remaining;
        for (const QString &cat : savedOrder) {
            if (!toMoveUnique.contains(cat))
                remaining.append(cat);
        }

        int insertAt = remaining.size();
        if (!targetCat.isEmpty()) {
            int idx = remaining.indexOf(targetCat);
            if (idx >= 0)
                insertAt = dropBelow ? idx + 1 : idx;
        }
        if (insertAt > remaining.size())
            insertAt = remaining.size();

        for (int i = 0; i < toMoveUnique.size(); ++i)
            remaining.insert(insertAt + i, toMoveUnique[i]);

        if (!remaining.isEmpty())
            snippetMgr->saveCategoryOrder(remaining);

        de->accept();
        QTimer::singleShot(0, this, [this]() { refreshCategoryTree(); });
        return true;
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
