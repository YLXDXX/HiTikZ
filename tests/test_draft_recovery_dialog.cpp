#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QTemporaryDir>
#include <QDebug>
#include "draft_recovery_dialog.h"

static int g_failed = 0;

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { \
            qDebug() << "FAIL:" << msg; \
            g_failed++; \
        } else { \
            qDebug() << "PASS:" << msg; \
        } \
    } while (0)

static void writeDraft(const QString &path, const QString &snippetId,
                       const QString &name, const QString &code,
                       const QString &description = QString())
{
    QJsonObject obj;
    obj["snippetId"] = snippetId;
    obj["code"] = code;
    obj["name"] = name;
    obj["description"] = description;
    obj["tags"] = QString();
    obj["packages"] = QString();
    obj["tikzLibraries"] = QString();
    obj["templateId"] = QString();
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    f.write(QJsonDocument(obj).toJson());
}

static QList<DraftRecoveryDialog::Draft> sampleDrafts(int count)
{
    QList<DraftRecoveryDialog::Draft> drafts;
    for (int i = 0; i < count; ++i) {
        DraftRecoveryDialog::Draft d;
        d.name = QStringLiteral("draft-%1").arg(i);
        d.code = QStringLiteral("\\draw (0,0) -- (%1,%1);").arg(i);
        drafts.append(d);
    }
    return drafts;
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));
    QApplication app(argc, argv);

    // --- loadDraftsFromDir ----------------------------------------------------
    {
        QTemporaryDir tmp;
        const QString dir = tmp.path();

        writeDraft(dir + "/a.json", "id-aaaa-1111", "Draft A",
                   "\\draw (0,0) -- (1,1);", "first draft");
        writeDraft(dir + "/b.json", QString(), QString(),
                   "\\draw (0,0) circle (1);");            // scratch, no name
        writeDraft(dir + "/c.json", "id-cccc", QString(), "   ");  // empty code
        {
            QFile f(dir + "/d.json");                       // corrupt file
            f.open(QIODevice::WriteOnly);
            f.write("{not json!");
        }

        const auto drafts = DraftRecoveryDialog::loadDraftsFromDir(dir);
        CHECK(drafts.size() == 2, "empty-code and corrupt drafts are filtered out");
        if (drafts.size() == 2) {
            CHECK(drafts[0].name == QStringLiteral("Draft A"), "draft name is read");
            CHECK(drafts[0].description == QStringLiteral("first draft"),
                  "draft description is read");
            CHECK(drafts[1].name == QStringLiteral("未命名草稿"),
                  "scratch draft gets fallback name");
            CHECK(drafts[1].filePath == dir + QStringLiteral("/b.json"),
                  "draft keeps its file path");
        }

        const auto none = DraftRecoveryDialog::loadDraftsFromDir(
            dir + QStringLiteral("/does-not-exist"));
        CHECK(none.isEmpty(), "missing directory yields no drafts");
    }

    // Fallback name from snippet id when name missing but id present
    {
        QTemporaryDir tmp;
        writeDraft(tmp.path() + "/x.json", "abcdefgh-rest-of-uuid", QString(),
                   "\\draw (0,0);");
        const auto drafts = DraftRecoveryDialog::loadDraftsFromDir(tmp.path());
        CHECK(drafts.size() == 1 && drafts[0].name == QStringLiteral("abcdefgh"),
              "unnamed draft with id falls back to id prefix");
    }

    // --- default selection state ----------------------------------------------
    {
        DraftRecoveryDialog dlg(sampleDrafts(3));
        CHECK(dlg.selectedIndices() == (QList<int>{0, 1, 2}),
              "all drafts checked by default");

        dlg.setAllChecked(false);
        CHECK(dlg.selectedIndices().isEmpty(), "deselect all clears selection");
        dlg.setAllChecked(true);
        CHECK(dlg.selectedIndices().size() == 3, "select all restores selection");

        const auto boxes = dlg.findChildren<QCheckBox *>();
        CHECK(boxes.size() == 3, "one checkbox per draft");
        if (boxes.size() == 3) {
            boxes[1]->setChecked(false);
            CHECK(dlg.selectedIndices() == (QList<int>{0, 2}),
                  "unchecking one draft removes exactly it");
        }
    }

    // --- the select-all / deselect-all push buttons ---------------------------
    {
        DraftRecoveryDialog dlg(sampleDrafts(2));
        QPushButton *selectAll = nullptr;
        QPushButton *deselectAll = nullptr;
        for (QPushButton *b : dlg.findChildren<QPushButton *>()) {
            if (b->text() == QStringLiteral("全选")) selectAll = b;
            else if (b->text() == QStringLiteral("取消全选")) deselectAll = b;
        }
        CHECK(selectAll && deselectAll, "select-all/deselect-all buttons exist");
        if (selectAll && deselectAll) {
            deselectAll->click();
            CHECK(dlg.selectedIndices().isEmpty(), "取消全选 button works");
            selectAll->click();
            CHECK(dlg.selectedIndices().size() == 2, "全选 button works");
        }
    }

    // --- result codes -----------------------------------------------------------
    // Regression: DiscardAll must be a distinct value. The old implementation
    // used QDialog::Rejected + 1, which equals QDialog::Accepted (1) — the
    // discard result was indistinguishable from "recover".
    {
        CHECK(int(DraftRecoveryDialog::DiscardAll) != int(QDialog::Accepted),
              "DiscardAll does not collide with Accepted");
        CHECK(int(DraftRecoveryDialog::DiscardAll) != int(QDialog::Rejected),
              "DiscardAll does not collide with Rejected");
    }

    // --- button wiring -----------------------------------------------------------
    // Regression: the Discard button has DestructiveRole, for which
    // QDialogButtonBox emits neither accepted() nor rejected(); the old code
    // listened on rejected(), so clicking 全部丢弃 did nothing at all.
    {
        DraftRecoveryDialog dlg(sampleDrafts(2));
        dlg.setConfirmDiscard(false);
        dlg.show();
        QPushButton *discardBtn = dlg.buttonBox()->button(QDialogButtonBox::Discard);
        CHECK(discardBtn != nullptr, "discard button exists");
        CHECK(discardBtn->text() == QStringLiteral("全部丢弃"),
              "discard button is labelled 全部丢弃");
        discardBtn->click();
        QApplication::processEvents();
        CHECK(dlg.result() == DraftRecoveryDialog::DiscardAll,
              "clicking 全部丢弃 closes the dialog with DiscardAll");
        CHECK(!dlg.isVisible(), "dialog closes after 全部丢弃");
    }

    // Ok → RecoverSelected
    {
        DraftRecoveryDialog dlg(sampleDrafts(2));
        dlg.show();
        dlg.buttonBox()->button(QDialogButtonBox::Ok)->click();
        QApplication::processEvents();
        CHECK(dlg.result() == DraftRecoveryDialog::RecoverSelected,
              "clicking 恢复所选 accepts the dialog");
    }

    // Cancel → Cancelled (keep drafts for next start)
    {
        DraftRecoveryDialog dlg(sampleDrafts(2));
        dlg.show();
        dlg.buttonBox()->button(QDialogButtonBox::Cancel)->click();
        QApplication::processEvents();
        CHECK(dlg.result() == DraftRecoveryDialog::Cancelled,
              "clicking 稍后处理 rejects the dialog");
    }

    if (g_failed > 0) {
        qDebug() << "\n" << g_failed << "test(s) failed!";
        return 1;
    }
    qDebug() << "\nAll draft recovery dialog tests passed!";
    return 0;
}
