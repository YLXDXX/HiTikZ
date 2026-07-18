#include "draft_recovery_dialog.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

DraftRecoveryDialog::DraftRecoveryDialog(const QList<Draft> &drafts, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("恢复草稿"));
    setMinimumSize(450, 300);
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *infoLabel = new QLabel(
        QStringLiteral("检测到 %1 个未保存的草稿。\n"
                       "请勾选要恢复的草稿（点击\u201c恢复所选\u201d后，未勾选的草稿将被丢弃）：")
            .arg(drafts.size()));
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    QScrollArea *scroll = new QScrollArea;
    QWidget *scrollWidget = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setContentsMargins(0, 0, 0, 0);

    for (const Draft &draft : drafts) {
        QString label = draft.name;
        if (!draft.description.isEmpty())
            label += QStringLiteral(" — %1").arg(draft.description.left(60));
        QCheckBox *cb = new QCheckBox(label);
        cb->setChecked(true);
        scrollLayout->addWidget(cb);
        m_checkboxes.append(cb);
    }
    scrollLayout->addStretch();

    QPushButton *selectAllBtn = new QPushButton(QStringLiteral("全选"));
    QPushButton *deselectAllBtn = new QPushButton(QStringLiteral("取消全选"));
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(selectAllBtn);
    btnRow->addWidget(deselectAllBtn);
    btnRow->addStretch();
    scrollLayout->addLayout(btnRow);

    connect(selectAllBtn, &QPushButton::clicked, this, [this]() { setAllChecked(true); });
    connect(deselectAllBtn, &QPushButton::clicked, this, [this]() { setAllChecked(false); });

    scroll->setWidget(scrollWidget);
    scroll->setWidgetResizable(true);
    layout->addWidget(scroll, 1);

    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(QStringLiteral("恢复所选"));
    m_buttonBox->button(QDialogButtonBox::Discard)->setText(QStringLiteral("全部丢弃"));
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("稍后处理"));
    layout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    // The Discard button has DestructiveRole: QDialogButtonBox emits neither
    // accepted() nor rejected() for it (those fire only for Accept/Yes and
    // Reject/No roles), so its clicked() signal must be wired explicitly —
    // this is exactly why the old in-dialog wiring made the button a no-op.
    connect(m_buttonBox->button(QDialogButtonBox::Discard), &QPushButton::clicked,
            this, [this]() {
        if (m_confirmDiscard) {
            QMessageBox::StandardButton ret = QMessageBox::warning(
                this, QStringLiteral("全部丢弃"),
                QStringLiteral("确定要丢弃全部 %1 个草稿吗？此操作不可恢复。")
                    .arg(m_checkboxes.size()),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (ret != QMessageBox::Yes)
                return;
        }
        done(DiscardAll);
    });
}

QList<int> DraftRecoveryDialog::selectedIndices() const
{
    QList<int> indices;
    for (int i = 0; i < m_checkboxes.size(); ++i) {
        if (m_checkboxes[i]->isChecked())
            indices.append(i);
    }
    return indices;
}

void DraftRecoveryDialog::setAllChecked(bool checked)
{
    for (QCheckBox *cb : m_checkboxes)
        cb->setChecked(checked);
}

void DraftRecoveryDialog::setConfirmDiscard(bool confirm)
{
    m_confirmDiscard = confirm;
}

QDialogButtonBox *DraftRecoveryDialog::buttonBox() const
{
    return m_buttonBox;
}

QList<DraftRecoveryDialog::Draft> DraftRecoveryDialog::loadDraftsFromDir(const QString &dirPath)
{
    QList<Draft> drafts;
    QDir d(dirPath);
    if (!d.exists())
        return drafts;

    const QStringList draftFiles =
        d.entryList(QStringList() << QStringLiteral("*.json"), QDir::Files, QDir::Name);
    for (const QString &fileName : draftFiles) {
        const QString filePath = d.filePath(fileName);
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
            continue;

        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (!doc.isObject())
            continue;

        const QJsonObject obj = doc.object();
        Draft info;
        info.filePath = filePath;
        info.snippetId = obj.value(QStringLiteral("snippetId")).toString();
        info.name = obj.value(QStringLiteral("name")).toString();
        info.code = obj.value(QStringLiteral("code")).toString();
        info.description = obj.value(QStringLiteral("description")).toString();
        info.tags = obj.value(QStringLiteral("tags")).toString();
        info.packages = obj.value(QStringLiteral("packages")).toString();
        info.tikzLibraries = obj.value(QStringLiteral("tikzLibraries")).toString();
        info.templateId = obj.value(QStringLiteral("templateId")).toString();

        if (info.name.isEmpty())
            info.name = info.snippetId.isEmpty() ? QStringLiteral("未命名草稿")
                                                 : info.snippetId.left(8);
        if (info.code.trimmed().isEmpty())
            continue;

        drafts.append(info);
    }
    return drafts;
}
