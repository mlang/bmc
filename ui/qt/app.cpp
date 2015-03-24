#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BrailleMusicEditor window;
    window.show();

    return app.exec();
}
