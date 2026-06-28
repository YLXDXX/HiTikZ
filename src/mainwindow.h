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
#include <QPdfDocument>
#include <QPdfView>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QComboBox>
#include <QTimer>
#include <QMap>
#include <QShortcut>
#include <QSettings>
#include <QToolButton>

#ifdef HAS_QHOTKEY
#include <QHotkey>
#endif

class SnippetManager;
class LatexCompiler;
class CodeEditor;
class SearchPanel;

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
    void zoomPdfIn();
    void zoomPdfOut();
    void fitPdfPage();
    void fitPdfWidth();
    void fitPdfHeight();
    void applyPdfZoomPreference();
    void applyShortcuts();
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

    QString snippetDataPath(const QString &id) const;

    void refreshTemplateCombo();

    SnippetManager *snippetMgr;
    LatexCompiler *compiler;
    SearchPanel *searchPanel;

    CodeEditor *codeEditor;
    QPlainTextEdit *logPanel;

    QWidget *rightPanel;
    QPdfView *pdfView;
    QPdfDocument *pdfDoc;
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
    QTimer *searchDebounceTimer = nullptr;
    bool m_pdfPanning = false;
    QPoint m_pdfPanStart;
    int m_pdfZoomPref = 0;
};
