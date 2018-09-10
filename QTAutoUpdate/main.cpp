#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    for(int i=1; i<argc; i++){
        qDebug() << argv[i];
    }

    w.saveExecuteArg(argc, argv);

    return a.exec();
}
