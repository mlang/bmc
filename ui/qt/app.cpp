#include "mainwindow.h"
#include <QApplication>

#include <locale>
#include <bmc/braille/text2braille.hpp>

int main(int argc, char *argv[])
{
  std::locale::global(std::locale(""));
  bmc::braille::set_default_table_from_locale();

    QApplication app(argc, argv);
    QApplication::setApplicationName("Braille Music Compiler");
    QApplication::setApplicationVersion(BMC_VERSION);
    BrailleMusicEditor window;
    window.show();

    return app.exec();
}
