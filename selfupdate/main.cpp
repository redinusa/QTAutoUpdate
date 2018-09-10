#include "mainwindow.h"
#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QThread::msleep(1000); // 希望updater 1s内能退出

    MainWindow w;
    //w.show();

    return a.exec();
}
