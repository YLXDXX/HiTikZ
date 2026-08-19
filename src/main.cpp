#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include "autostart_manager.h"
#include "mainwindow.h"
#include "project_packager.h"
#include "snippet_manager.h"

#ifndef APP_VERSION
#define APP_VERSION "0.0"
#endif

int main(int argc, char *argv[]) {
    // CLI mode: "hitikz pack ..." — packs the 「复制链接」 pictures into a
    // LaTeX project directory and rewrites \includegraphics references.
    // Runs without a GUI (QCoreApplication only), so it works over SSH too.
    if (argc > 1 && QString::fromLocal8Bit(argv[1]) == QStringLiteral("pack")) {
        QStringList args;
        for (int i = 2; i < argc; ++i)
            args.append(QString::fromLocal8Bit(argv[i]));
        return ProjectPackager::runCli(args);
    }

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
    parser.setApplicationDescription(
        QStringLiteral("HiTikZ - TikZ 代码合集管理器\n\n"
                       "子命令:\n"
                       "  pack  读取 .tex 文档引用，将「复制链接」的图片按需复制进 LaTeX 文档项目\n"
                       "        并改写 \\includegraphics 引用（用法: hitikz pack --help）"));
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
