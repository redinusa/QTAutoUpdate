#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString appDir = QApplication::applicationDirPath();

    QDir dir(appDir);

    QString updater = "";
    foreach (QFileInfo info, dir.entryInfoList()){
        if(!info.isDir() && info.fileName().contains("QTAutoUpdate")){
            updater = info.fileName();
        }
    }

    if(updater.isEmpty())
        exit(0);

    QFile file(appDir + "/" + updater);

    QFile file1(appDir + "/qtautoupdate_tmp.tmp");
    if(file1.exists()){
        file.remove();
        file1.rename(appDir + "/" + updater);
    }

    exit(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
