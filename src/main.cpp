#include <QApplication>
#include <QStandardPaths>
#include "mainwindow.h"
#include "snippet_manager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qRegisterMetaType<Snippet>("Snippet");

    app.setOrganizationName("HiTikZ");
    app.setApplicationName("TikzManager");
    app.setApplicationVersion("1.0");

    MainWindow window;
    window.show();
    return app.exec();
}
