#include <QApplication>
#include <QStandardPaths>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setOrganizationName("HiTikZ");
    app.setApplicationName("TikzManager");
    app.setApplicationVersion("1.0");

    MainWindow window;
    window.resize(1200, 800);
    window.show();
    return app.exec();
}
