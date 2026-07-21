#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QPlainTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QStandardItemModel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QComboBox>
#include <QTimer>
#include <QMap>
#include <QShortcut>
#include <QSettings>
#include <QToolButton>
#include <QTabWidget>
#include <QAtomicInt>
#include <QMutex>
#include "snippet_manager.h"

#ifdef HAS_QHOTKEY
#include <QHotkey>
#endif

class LatexCompiler;
class CodeEditor;
class SearchPanel;
class PdfPreviewWidget;

struct ParamInfo {
    QString name;
    QString defaultValue;
    QLineEdit *edit;
};

struct TabUiState {
    QString name;
    QString desc;
    QString tags;
    QString packages;
    QString tikzLibraries;
    QString templateId;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void generateAllPreviews();
    void factoryReset();
    void applyGlobalHotkey();

    // Returns true if the tab at `index` has unsaved changes (code or, for the
    // active tab, metadata). Exposed for tests.
    bool tabHasUnsavedChanges(int index) const;

signals:
    void batchPreviewFinished();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void setupConnections();
    void refreshSearch();
    void loadSnippetIntoEditor(const QString &id);
    void saveCurrentSnippet();
    static QString resolveParamsFromCode(const QString &code);
    void applyShortcuts();
    void applyAppearanceSettings();
    void copyFullDocument();
    // Isolate metadata edits across tabs: save the current tab's UI state before
    // switching away, and restore it when switching back so unsaved edits survive.
    void saveCurrentTabUiState();
    void restoreTabUiState(const QString &sid);
    void updateTabUiState();
    // Push the current metadata fields (extra packages + TikZ libraries) into
    // the active editor's document state so completion/highlighting reflect
    // libraries added via the UI, not just \usetikzlibrary{...} in the code.
    void syncDocStateLibraries();
    // Raw content of a LaTeX template (empty if the id is empty/invalid); its
    // \usepackage/\usetikzlibrary lines feed completion too.
    static QString templateContentFor(const QString &templateId);
    void setFitPageChecked(bool checked);
    void setFitWidthChecked(bool checked);
    void setFitHeightChecked(bool checked);
    void refreshCategoryTree();
    void onCurrentSnippetChanged();
    void jumpToErrorLine(const QString &logText);
    void parseParams();
    void performParseParams();
    void clearParams();
    QString applyParams(const QString &code);
    void savePreviewData(const QString &pdfPath, const QString &snippetId);
    void loadPreviewForSnippet(const QString &id);
    void clearPdfPreview();
    void setFormattedLog(bool success, const QString &command, const QString &log, int userCodeStartLine);
    void handleLogDoubleClick();
    void checkSystemDependencies();
    void updateFitActionStates();
    void startAutoSave();
    void performAutoSave();
    void recoverDrafts();
    void clearDraft(int tabIndex = -1);
    void clearAllDrafts();
    void centerOnScreen();
    void saveWindowGeometry();
    void restoreWindowGeometry();

    QString snippetDataPath(const QString &id) const;

    void refreshTemplateCombo();

    CodeEditor *currentEditor() const;
    QString currentTabSnippetId() const;
    int findTabForSnippet(const QString &id) const;
    bool maybeCloseTab(int index);
    bool isSnippetDirty(const QString &sid, CodeEditor *editor) const;
    void updateTabTitle(int index, const QString &title);
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void setEditorForTab(int index);
    void createNewTab(const QString &snippetId, const QString &code, const QString &title);
    void connectEditorSignals(CodeEditor *editor);
    // Enable undo/redo only when the current editor's document actually has
    // something to undo/redo (disabled with no tabs or pristine history).
    void updateUndoRedoActions();

    SnippetManager *snippetMgr;
    LatexCompiler *compiler;
    SearchPanel *searchPanel;

    QTabWidget *tabWidget;
    QPlainTextEdit *logPanel;

    QWidget *rightPanel;
    PdfPreviewWidget *pdfPreview;
    QLineEdit *nameEdit;
    QTextEdit *descEdit;
    QLineEdit *tagsEdit;
    QLineEdit *packagesEdit;
    QLineEdit *tikzLibrariesEdit;
    QComboBox *templateCombo;

    QScrollArea *paramsScrollArea;
    QWidget *paramsWidget;
    QVBoxLayout *paramsLayout;
    QList<ParamInfo> currentParams;

    QShortcut *copyCodeShortcut;
    QShortcut *copyPngShortcut;
    QShortcut *copySvgShortcut;
    QShortcut *compileShortcut;
    QShortcut *applyParamsShortcut;
    QShortcut *saveShortcut;
    QShortcut *closeTabShortcut;
#ifdef HAS_QHOTKEY
    QHotkey *globalHotkey = nullptr;
#endif

    QAction *fitPageAct;
    QAction *fitWidthAct;
    QAction *fitHeightAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *compileAct;
    QAction *applyParamsAct;
    QAction *saveAct;
    QAction *undoAct = nullptr;
    QAction *redoAct = nullptr;
    QAction *duplicateAct = nullptr;
    QAction *openPdfExternalAct = nullptr;

    QAction *forceStopAct;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    QString currentSnippetId;
    bool m_compiling = false;
    bool m_batchGenerating = false;
    bool m_forceQuit = false;
    // Startup tasks that must not pop dialogs while the window is hidden
    // (e.g. --hidden autostart); they run once the window is first shown.
    bool m_pendingDependencyCheck = false;
    bool m_pendingDraftRecovery = false;
    bool m_clipboardPngPending = false;
    bool m_clipboardSvgPending = false;
    int m_loadingDepth = 0;
    int m_userCodeStartLine = 1;
    QString m_lastSavedCode;
    QMap<QString, QMap<QString, QString>> m_perSnippetParamValues;
    int m_previewTotal = 0;
    QAtomicInt m_batchCompleted{0};
    QAtomicInt m_batchSubmitted{0};
    QAtomicInt m_batchCancelFlag{0};
    QList<QPair<Snippet, QString>> m_batchFailures;
    QMutex m_batchMutex;
    // Per-tab metadata state so unsaved edits survive tab switches.
    QMap<QString, TabUiState> m_tabUiStates;
    // Snippet id whose metadata is currently shown in the shared right-panel
    // widgets. Used to persist the *departing* tab's edits under the correct id
    // when switching tabs (QTabWidget::currentIndex() already points at the new
    // tab by the time currentChanged fires).
    QString m_uiStateSnippetId;
    QTimer *searchDebounceTimer = nullptr;
    QTimer *autoSaveTimer = nullptr;
    QTimer *m_parseParamsTimer = nullptr;
    QLabel *m_compileStatusLabel = nullptr;
    QTimer *m_compileStatusTimer = nullptr;

private slots:
    void onBatchTaskFinished(const Snippet &snippet, bool success, const QString &pdfPath, const QString &log);

private:
    void showBatchPreviewSummary();
};
