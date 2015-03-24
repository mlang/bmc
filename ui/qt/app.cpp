#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Braille Music Compiler");
    QApplication::setApplicationVersion(BMC_VERSION);
    BrailleMusicEditor window;
    window.show();

    return app.exec();
}
