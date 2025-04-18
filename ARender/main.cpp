#include "mainwindow.h"
#include <qfontdatabase.h>
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFontDatabase::addApplicationFont(":/fonts/font_awesome_6_regular_free.otf"); // Add font awesome font to application
    QFontDatabase::addApplicationFont(":/fonts/font_awesome_6_solid_free.otf"); // Add font awesome font to application
    QFontDatabase::addApplicationFont(":/fonts/corbel.ttf"); // Add corbel font to application
    QFontDatabase::addApplicationFont(":/fonts/Deng.ttf"); // Add Deng Xian font to application
    QFontDatabase::addApplicationFont(":/fonts/Dengb.ttf"); // Add Deng Xian Bold font to application
    QFontDatabase::addApplicationFont(":/fonts/Dengl.ttf"); // Add Deng Xian Light font to application
    MainWindow w;
    w.setMouseTracking(true);
    w.resize(1250, 750);
    w.show();
    return a.exec();
}
