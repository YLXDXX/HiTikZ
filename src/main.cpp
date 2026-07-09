#include <QApplication>
#include <QStandardPaths>
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

    MainWindow window;
    window.show();
    return app.exec();
}
