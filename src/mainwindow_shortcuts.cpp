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

void MainWindow::applyShortcuts()
{
    QSettings settings("HiTikZ", "TikzManager");

    auto setShortcut = [&](QShortcut *sc, const QString &key, const QString &defaultVal = QString()) {
        QString val = settings.value(key, defaultVal).toString();
        sc->setKey(val.isEmpty() ? QKeySequence() : QKeySequence(val));
    };

    setShortcut(copyCodeShortcut, "shortcuts/copyCode");
    setShortcut(copyPngShortcut, "shortcuts/copyPng");
    setShortcut(copySvgShortcut, "shortcuts/copySvg");
    setShortcut(compileShortcut, "shortcuts/compile", "F6");
    setShortcut(applyParamsShortcut, "shortcuts/applyParams");
    setShortcut(saveShortcut, "shortcuts/save", "Ctrl+S");
    setShortcut(closeTabShortcut, "shortcuts/closeTab", "Ctrl+W");
}

void MainWindow::applyGlobalHotkey()
{
    QSettings settings("HiTikZ", "TikzManager");
    QString keyStr = settings.value("shortcuts/globalHotkey", "").toString();

#ifdef HAS_KGLOBALACCEL
    KdeGlobalShortcut *ks = KdeGlobalShortcut::instance();
    ks->disconnect(this);
    if (keyStr.isEmpty()) {
        ks->unregisterShortcut("toggle_window");
        return;
    }

    connect(ks, &KdeGlobalShortcut::activated, this, [this](const QString &id) {
        if (id == "toggle_window") {
            if (isVisible() && !isMinimized()) {
                saveWindowGeometry();
                hide();
            } else {
                restoreWindowGeometry();
                show();
                raise();
                activateWindow();
                searchPanel->setFocus();
            }
        }
    });
    ks->registerShortcut("toggle_window", QStringLiteral("显示/隐藏窗口"), keyStr);
    return;
#endif

#ifdef HAS_QHOTKEY
    if (globalHotkey) {
        globalHotkey->disconnect(this);
        delete globalHotkey;
        globalHotkey = nullptr;
    }
    if (keyStr.isEmpty()) return;

    QKeySequence ks(keyStr);
    if (ks.isEmpty()) return;

    globalHotkey = new QHotkey(ks, true, this);
    if (globalHotkey->isRegistered()) {
        connect(globalHotkey, &QHotkey::activated, this, [this]() {
            if (isVisible() && !isMinimized()) {
                saveWindowGeometry();
                hide();
            } else {
                restoreWindowGeometry();
                show();
                raise();
                activateWindow();
                searchPanel->setFocus();
            }
        });
    }
#endif
}
