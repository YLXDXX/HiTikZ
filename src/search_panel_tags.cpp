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

int SearchPanel::thumbnailCount() const
{
    return thumbnailModel ? thumbnailModel->rowCount() : 0;
}

void SearchPanel::setTagSelected(const QString &tag, bool selected)
{
    if (selected)
        m_selectedTags.insert(tag);
    else
        m_selectedTags.remove(tag);
    refreshSearch();
}

void SearchPanel::refreshTagFilter()
{
    // Compute the current tag universe first, WITHOUT touching the existing
    // widgets. Saving a snippet usually leaves the tag set unchanged; rebuilding
    // the whole strip in that case makes every tag button flash (fully expanded
    // for one frame, then re-collapsed by the 150ms timer). Only rebuild when
    // the tag set actually changed.
    QSet<QString> allTags;
    // false = only read meta.json (tags are stored in metadata, no need for .tex)
    QList<Snippet> all = snippetMgr->getAllSnippets(false);
    QList<Snippet> presets = snippetMgr->getAllPresets(false);
    all.append(presets);
    for (const Snippet &s : all) {
        for (const QString &tag : s.tags) {
            if (!tag.trimmed().isEmpty())
                allTags.insert(tag.trimmed());
        }
    }

    QStringList newTagNames = allTags.values();
    newTagNames.sort(Qt::CaseInsensitive);

    // Prune any selected tags that no longer exist on any snippet. When a tag is
    // removed from the last snippet that carried it, it must also drop out of the
    // active filter — otherwise the thumbnail list stays filtered against a tag
    // nothing has (showing nothing) and there is no longer a button to unselect
    // it. If we pruned anything, re-run the search so the list updates even when
    // the caller already searched before refreshing the tag strip.
    bool prunedSelection = false;
    for (const QString &sel : m_selectedTags.values()) {
        if (!allTags.contains(sel)) {
            m_selectedTags.remove(sel);
            prunedSelection = true;
        }
    }

    // Nothing changed and the strip is already built: leave the widgets (and
    // their collapsed state) exactly as they are — no flicker. A pruned
    // selection always implies the tag set shrank, so it never reaches here.
    if (!prunedSelection && newTagNames == m_allTagNames
        && m_tagButtonContainer) {
        return;
    }

    m_allTagNames = newTagNames;

    QLayout *existingLayout = tagFilterWidget->layout();
    QLayoutItem *child;
    while ((child = existingLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
    m_tagButtonContainer = nullptr;
    m_tagFlowLayout = nullptr;
    m_moreTagsBtn = nullptr;

    if (m_allTagNames.isEmpty()) {
        if (prunedSelection)
            refreshSearch();
        return;
    }

    m_tagButtonContainer = new QWidget;
    m_tagFlowLayout = new FlowLayout(m_tagButtonContainer, 0, 4, 2);

    for (const QString &tag : m_allTagNames) {
        QPushButton *btn = new QPushButton(tag);
        btn->setCheckable(true);
        btn->setChecked(m_selectedTags.contains(tag));
        btn->setFlat(true);
        btn->setStyleSheet(QStringLiteral(
            "QPushButton { padding: 2px 8px; border: 1px solid #ccc; border-radius: 10px; background: #eee; }"
            "QPushButton:checked { background: #4a90d9; color: white; border-color: #4a90d9; }"));
        connect(btn, &QPushButton::toggled, this, [this, tag](bool checked) {
            if (checked)
                m_selectedTags.insert(tag);
            else
                m_selectedTags.remove(tag);
            refreshSearch();
        });
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        m_tagFlowLayout->addWidget(btn);
    }

    m_moreTagsBtn = new QPushButton(QStringLiteral("更多标签..."));
    m_moreTagsBtn->setFlat(true);
    m_moreTagsBtn->setStyleSheet(QStringLiteral(
        "QPushButton { padding: 2px 8px; border: 1px solid #ccc; border-radius: 10px; color: #4a90d9; }"
        "QPushButton:hover { text-decoration: underline; }"));
    connect(m_moreTagsBtn, &QPushButton::clicked, this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle(QStringLiteral("选择标签"));
        dlg.setMinimumSize(350, 350);
        QVBoxLayout *dlgLayout = new QVBoxLayout(&dlg);

        QLineEdit *filter = new QLineEdit;
        filter->setPlaceholderText(QStringLiteral("搜索标签..."));
        filter->setClearButtonEnabled(true);
        dlgLayout->addWidget(filter);

        QScrollArea *scroll = new QScrollArea;
        QWidget *scrollWidget = new QWidget;
        FlowLayout *flowLayout = new FlowLayout(scrollWidget, 0, 4, 4);

        QList<QPushButton *> tagButtons;
        for (const QString &tag : m_allTagNames) {
            QPushButton *btn = new QPushButton(tag);
            btn->setCheckable(true);
            btn->setChecked(m_selectedTags.contains(tag));
            btn->setFlat(true);
            btn->setStyleSheet(QStringLiteral(
                "QPushButton { padding: 3px 10px; border: 1px solid #ccc; border-radius: 10px; background: #eee; }"
                "QPushButton:checked { background: #4a90d9; color: white; border-color: #4a90d9; }"));
            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            connect(btn, &QPushButton::toggled, this, [this, tag](bool checked) {
                if (checked)
                    m_selectedTags.insert(tag);
                else
                    m_selectedTags.remove(tag);
                refreshSearch();
            });
            flowLayout->addWidget(btn);
            tagButtons.append(btn);
        }

        scroll->setWidget(scrollWidget);
        scroll->setWidgetResizable(true);
        dlgLayout->addWidget(scroll, 1);

        connect(filter, &QLineEdit::textChanged, this, [&tagButtons](const QString &text) {
            for (QPushButton *btn : tagButtons) {
                btn->setVisible(text.isEmpty() || btn->text().contains(text, Qt::CaseInsensitive));
            }
        });

        QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        dlgLayout->addWidget(btnBox);

        dlg.exec();
        refreshTagFilter();
    });
    m_tagFlowLayout->addWidget(m_moreTagsBtn);

    existingLayout->addWidget(m_tagButtonContainer);

    if (m_tagCollapseTimer)
        m_tagCollapseTimer->start();

    // A pruned selection changes the effective filter; refresh the results so
    // the thumbnail list reflects the now-smaller tag set.
    if (prunedSelection)
        refreshSearch();
}

void SearchPanel::applyTagRowCollapse()
{
    if (m_inTagCollapse) return;
    if (!m_tagButtonContainer || !m_tagFlowLayout || !m_moreTagsBtn) return;

    m_inTagCollapse = true;

    // Show all tags temporarily to measure the true row count,
    // avoiding artificial deflation from previously hidden widgets.
    for (int i = 0; i < m_tagFlowLayout->count(); ++i) {
        QLayoutItem *it = m_tagFlowLayout->itemAt(i);
        if (it && it->widget() && it->widget() != m_moreTagsBtn)
            it->widget()->setVisible(true);
    }

    m_tagFlowLayout->invalidate();
    m_tagFlowLayout->activate();

    int totalRows = m_tagFlowLayout->rowCount();
    bool needsCollapse = totalRows > kMaxTagRows;

    m_moreTagsBtn->setVisible(needsCollapse);

    if (!needsCollapse) {
        m_inTagCollapse = false;
        return;
    }

    int rows = 0;
    int prevY = -1;

    for (int i = 0; i < m_tagFlowLayout->count(); ++i) {
        QLayoutItem *it = m_tagFlowLayout->itemAt(i);
        if (!it || !it->widget()) continue;
        QWidget *w = it->widget();
        if (w == m_moreTagsBtn) continue;

        int widgetY = w->geometry().y();
        if (widgetY != prevY) {
            rows++;
            prevY = widgetY;
            if (rows > kMaxTagRows) {
                w->setVisible(false);
                continue;
            }
        }
        w->setVisible(rows <= kMaxTagRows);
    }

    m_inTagCollapse = false;
}
