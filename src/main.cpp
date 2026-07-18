#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include "autostart_manager.h"
#include "mainwindow.h"
#include "snippet_manager.h"

#ifndef APP_VERSION
#define APP_VERSION "0.0"
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qRegisterMetaType<Snippet>("Snippet");

    app.setOrganizationName("HiTikZ");
    app.setApplicationName("TikzManager");
    app.setApplicationVersion(QStringLiteral(APP_VERSION));
    app.setDesktopFileName(QStringLiteral("hitikz"));

    // Tray-resident app: closing/hiding the main window must not quit, and a
    // dialog shown while the window is hidden (dependency warning, draft
    // recovery) must not become "the last window" whose closing exits the
    // app. Real quitting goes through MainWindow::closeEvent() which calls
    // QApplication::quit() explicitly.
    app.setQuitOnLastWindowClosed(false);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("HiTikZ - TikZ 代码合集管理器"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption hiddenOpt(
        QStringList() << QStringLiteral("hidden") << QStringLiteral("minimized"),
        QStringLiteral("启动时不显示主窗口，仅驻留系统托盘（用于开机自启动）。"));
    parser.addOption(hiddenOpt);
    parser.process(app);

    // Legacy autostart entries launched the app without --hidden, popping the
    // main window on every login; upgrade them in place.
    AutostartManager::migrateEntryToHidden();

    MainWindow window;
    if (!parser.isSet(hiddenOpt))
        window.show();
    return app.exec();
}
