#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    setenv("QT_HARFBUZZ", "old", 1);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
