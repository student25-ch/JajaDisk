/* JajaDisk - Copyright (C) 2026 杨子桐 - AGPL-3.0 */
#include <QApplication>
#include "MainWindow.h"
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName("JajaDisk");
    app.setApplicationVersion("0.3");
    app.setStyle("Fusion");
    MainWindow w;
    w.show();
    return app.exec();
}