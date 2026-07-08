#include "mainwindow.h"
#include "search_panel.h"
#include "snippet_manager.h"
#include "latex_compiler.h"
#include "code_editor.h"
#include "tikz_document_state.h"
#include "settings_dialog.h"
#ifdef HAS_KGLOBALACCEL
#include "kde_global_shortcut.h"
#endif
#include "pdf_preview_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QInputDialog>
#include <QSplitter>
#include <QApplication>
#include <QStatusBar>
#include <QHeaderView>
#include <QIcon>
#include <QFileDialog>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QFileInfo>
#include <QClipboard>
#include <QImage>
#include <QShortcut>
#include <QMenu>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QEventLoop>
#include <QTimer>
#include <QMimeData>
#include <QDataStream>
#include <QDropEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QTabBar>
#include <QCheckBox>
#include <memory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QScreen>
#include <QtConcurrent>
#include <QThreadPool>
#include "mainwindow_internal.h"

CodeEditor *MainWindow::currentEditor() const
{
    if (!tabWidget || tabWidget->count() == 0)
        return nullptr;
    return qobject_cast<CodeEditor*>(tabWidget->currentWidget());
}

QString MainWindow::currentTabSnippetId() const
{
    int idx = tabWidget ? tabWidget->currentIndex() : -1;
    if (idx < 0) return QString();
    return tabWidget->tabBar()->tabData(idx).toString();
}

int MainWindow::findTabForSnippet(const QString &id) const
{
    if (id.isEmpty() || !tabWidget) return -1;
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabBar()->tabData(i).toString() == id)
            return i;
    }
    return -1;
}

void MainWindow::updateTabTitle(int index, const QString &title)
{
    if (index < 0 || index >= tabWidget->count()) return;
    tabWidget->setTabText(index, title);
}

void MainWindow::connectEditorSignals(CodeEditor *editor)
{
    connect(editor, &QPlainTextEdit::textChanged, this, [this]() {
        if (m_loadingDepth == 0)
            onCurrentSnippetChanged();
    });
}

void MainWindow::createNewTab(const QString &snippetId, const QString &code,
                              const QString &title)
{
    if (!tabWidget) return;

    CodeEditor *editor = new CodeEditor;
    editor->setTabStopDistance(4 * editor->fontMetrics().horizontalAdvance(' '));
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    connectEditorSignals(editor);

    tabWidget->blockSignals(true);
    int idx = tabWidget->addTab(editor, title);
    tabWidget->tabBar()->setTabData(idx, snippetId);
    tabWidget->blockSignals(false);
    tabWidget->setCurrentIndex(idx);

    if (!code.isNull()) {
        m_loadingDepth++;
        const QSignalBlocker blocker(editor);
        editor->setPlainText(code);
        m_loadingDepth--;
    }

    applyAppearanceSettings();
}

void MainWindow::setEditorForTab(int index)
{
    // Snapshot the metadata widgets before they are overwritten so the tab we
    // are leaving retains its unsaved edits.
    saveCurrentTabUiState();

    m_loadingDepth++;

    QString sid = (index >= 0 && index < tabWidget->count())
        ? tabWidget->tabBar()->tabData(index).toString()
        : QString();
    currentSnippetId = sid;

    if (sid.isEmpty()) {
        nameEdit->clear();
        descEdit->clear();
        tagsEdit->clear();
        packagesEdit->clear();
        tikzLibrariesEdit->clear();
        loadPreviewForSnippet(QString());
        clearParams();
        if (CodeEditor *ed = currentEditor())
            m_lastSavedCode = ed->toPlainText();
        else
            m_lastSavedCode.clear();
    } else {
        // Restore per-tab UI state first (may contain unsaved edits left over
        // from a prior tab switch), then fall back to on-disk data.
        if (m_tabUiStates.contains(sid)) {
            restoreTabUiState(sid);
        } else {
            Snippet s = snippetMgr->loadSnippet(sid);
            if (!s.id.isEmpty()) {
                nameEdit->setText(s.name);
                descEdit->setPlainText(s.description);
                tagsEdit->setText(s.tags.join(", "));
                packagesEdit->setText(s.packages);
                tikzLibrariesEdit->setText(s.tikzLibraries);

                if (!s.templateId.isEmpty()) {
                    int ti = templateCombo->findData(s.templateId);
                    if (ti >= 0) templateCombo->setCurrentIndex(ti);
                }
                loadPreviewForSnippet(sid);
                m_lastSavedCode = s.code;
            }
            return;
        }

        loadPreviewForSnippet(sid);
        m_lastSavedCode = snippetMgr->loadSnippet(sid).code;
    }

    m_loadingDepth--;
    performParseParams();
}

void MainWindow::onTabChanged(int index)
{
    if (index < 0) {
        currentSnippetId.clear();
        nameEdit->clear();
        descEdit->clear();
        tagsEdit->clear();
        packagesEdit->clear();
        tikzLibrariesEdit->clear();
        clearPdfPreview();
        clearParams();
        return;
    }
    setEditorForTab(index);
}

bool MainWindow::isSnippetDirty(const QString &sid, CodeEditor *editor) const
{
    if (!editor) return false;

    // Unsaved scratch tab (no backing snippet): dirty if it has any content.
    if (sid.isEmpty())
        return !editor->toPlainText().trimmed().isEmpty();

    Snippet saved = snippetMgr->loadSnippet(sid);
    if (saved.id.isEmpty())
        return true; // backing snippet vanished but the tab still has content

    if (editor->toPlainText() != saved.code)
        return true;

    // The metadata widgets (name/description/tags/packages/libs/template) are
    // shared across tabs and only reflect the *currently active* tab. Comparing
    // them for a background tab would be meaningless, so only do so when this
    // editor is the active one.
    if (editor == currentEditor()) {
        if (nameEdit->text() != saved.name) return true;
        if (descEdit->toPlainText() != saved.description) return true;
        if (packagesEdit->text() != saved.packages) return true;
        if (tikzLibrariesEdit->text() != saved.tikzLibraries) return true;
        if (tagsEdit->text() != saved.tags.join(QStringLiteral(", "))) return true;
        if (!saved.templateId.isEmpty()
            && templateCombo->currentData().toString() != saved.templateId)
            return true;
    }
    return false;
}

bool MainWindow::tabHasUnsavedChanges(int index) const
{
    if (!tabWidget || index < 0 || index >= tabWidget->count())
        return false;
    QString sid = tabWidget->tabBar()->tabData(index).toString();
    CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(index));
    return isSnippetDirty(sid, editor);
}

bool MainWindow::maybeCloseTab(int index)
{
    if (index < 0 || index >= tabWidget->count()) return true;

    QString sid = tabWidget->tabBar()->tabData(index).toString();
    CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(index));
    if (!editor) return true;

    bool hasUnsaved = isSnippetDirty(sid, editor);

    if (hasUnsaved) {
        tabWidget->setCurrentIndex(index);

        QMessageBox msgBox(this);
        msgBox.setWindowTitle(QStringLiteral("未保存的更改"));
        msgBox.setText(QStringLiteral("当前片段 \"%1\" 有未保存的更改。\n\n是否保存？")
                           .arg(tabWidget->tabText(index)));
        msgBox.setIcon(QMessageBox::Warning);
        QPushButton *saveBtn = msgBox.addButton(QStringLiteral("保存"), QMessageBox::AcceptRole);
        QPushButton *discardBtn = msgBox.addButton(QStringLiteral("不保存"), QMessageBox::DestructiveRole);
        QPushButton *cancelBtn = msgBox.addButton(QStringLiteral("取消"), QMessageBox::RejectRole);
        msgBox.setDefaultButton(saveBtn);
        msgBox.exec();

        QAbstractButton *clicked = msgBox.clickedButton();
        if (clicked == saveBtn) {
            saveCurrentSnippet();
            if (sid.isEmpty() && !editor->toPlainText().trimmed().isEmpty()) {
                QDialog dlg(this);
                dlg.setWindowTitle(QStringLiteral("保存新片段"));
                QFormLayout *form = new QFormLayout(&dlg);
                QLineEdit *nameEdit = new QLineEdit;
                QLineEdit *catEdit = new QLineEdit;
                catEdit->setPlaceholderText(QStringLiteral("如: 数学/几何"));
                form->addRow(QStringLiteral("片段名称:"), nameEdit);
                form->addRow(QStringLiteral("分类:"), catEdit);
                QDialogButtonBox *btnBox = new QDialogButtonBox(
                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                form->addRow(btnBox);
                connect(btnBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
                connect(btnBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
                if (dlg.exec() == QDialog::Accepted && !nameEdit->text().isEmpty()) {
                    QString id = snippetMgr->createSnippet(nameEdit->text(), catEdit->text());
                    tabWidget->tabBar()->setTabData(index, id);
                    tabWidget->setTabText(index, nameEdit->text());
                    currentSnippetId = id;
                    saveCurrentSnippet();
                } else {
                    return false;
                }
            }
        } else if (clicked == discardBtn) {
            // Clean up draft and per-tab UI state on discard.
            clearDraft(index);
            if (!sid.isEmpty()) m_tabUiStates.remove(sid);
        } else {
            return false;
        }
    }

    if (sid == currentSnippetId || currentSnippetId.isEmpty()) {
        currentSnippetId.clear();
    }

    return true;
}

void MainWindow::onTabCloseRequested(int index)
{
    if (!maybeCloseTab(index)) return;

    CodeEditor *editor = qobject_cast<CodeEditor*>(tabWidget->widget(index));
    tabWidget->removeTab(index);
    if (editor)
        editor->deleteLater();
}
