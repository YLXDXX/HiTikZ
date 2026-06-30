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

#ifdef HAS_QHOTKEY
#include <QHotkey>
#endif

class SnippetManager;
class LatexCompiler;
class CodeEditor;
class SearchPanel;
class PdfPreviewWidget;

struct ParamInfo {
    QString name;
    QString defaultValue;
    QLineEdit *edit;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void generateAllPreviews();
    void factoryReset();
    void applyGlobalHotkey();

protected:
    void closeEvent(QCloseEvent *event) override;
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
    void setFitPageChecked(bool checked);
    void setFitWidthChecked(bool checked);
    void setFitHeightChecked(bool checked);
    void refreshCategoryTree();
    void onCurrentSnippetChanged();
    void jumpToErrorLine(const QString &logText);
    void parseParams();
    void clearParams();
    QString applyParams(const QString &code);
    void savePreviewData(const QString &pdfPath, const QString &snippetId);
    void loadPreviewForSnippet(const QString &id);
    void clearPdfPreview();
    void setFormattedLog(const QString &log);
    void handleLogDoubleClick();
    void checkSystemDependencies();
    void updateFitActionStates();
    void startAutoSave();
    void performAutoSave();
    void clearDraft();
    void clearAllDrafts();

    QString snippetDataPath(const QString &id) const;

    void refreshTemplateCombo();

    CodeEditor *currentEditor() const;
    QString currentTabSnippetId() const;
    int findTabForSnippet(const QString &id) const;
    bool maybeCloseTab(int index);
    void updateTabTitle(int index, const QString &title);
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void setEditorForTab(int index);
    void createNewTab(const QString &snippetId, const QString &code, const QString &title);
    void connectEditorSignals(CodeEditor *editor);

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

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    QString currentSnippetId;
    bool m_batchGenerating = false;
    bool m_forceQuit = false;
    bool m_loadingTab = false;
    QTimer *searchDebounceTimer = nullptr;
    QTimer *autoSaveTimer = nullptr;
};
