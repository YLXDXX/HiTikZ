#pragma once
#include <QDialog>
#include <QList>
#include <QString>

class QCheckBox;
class QDialogButtonBox;

// Startup dialog offering to restore auto-saved drafts left behind after a
// crash / forced termination. Extracted from MainWindow so the button wiring
// and result codes are unit-testable.
class DraftRecoveryDialog : public QDialog {
    Q_OBJECT
public:
    struct Draft {
        QString filePath;
        QString snippetId;
        QString name;
        QString code;
        QString description;
        QString tags;
        QString packages;
        QString tikzLibraries;
        QString templateId;
    };

    // Distinct result codes. Note: QDialog::Rejected == 0 and
    // QDialog::Accepted == 1, so DiscardAll must be Accepted + 1 — the old
    // in-place implementation used Rejected + 1, which collides with Accepted.
    enum ResultCode {
        Cancelled = QDialog::Rejected,        // keep drafts, ask again next start
        RecoverSelected = QDialog::Accepted,  // restore checked drafts
        DiscardAll = QDialog::Accepted + 1    // delete every draft
    };

    explicit DraftRecoveryDialog(const QList<Draft> &drafts, QWidget *parent = nullptr);

    // Indices (into the constructor's draft list) that are currently checked.
    QList<int> selectedIndices() const;
    void setAllChecked(bool checked);
    // Tests disable the modal "really discard?" confirmation.
    void setConfirmDiscard(bool confirm);
    QDialogButtonBox *buttonBox() const;

    // Reads all *.json drafts in `dirPath`; skips unparsable files and drafts
    // whose code is effectively empty; fills in a fallback display name.
    static QList<Draft> loadDraftsFromDir(const QString &dirPath);

private:
    QList<QCheckBox *> m_checkboxes;
    QDialogButtonBox *m_buttonBox = nullptr;
    bool m_confirmDiscard = true;
};
