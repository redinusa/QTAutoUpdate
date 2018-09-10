#include <QFileDialog>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "workthread.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(400, 300);
    ui->centralWidget->setStyleSheet("border-image:url(:/applicationwindow-background.png)");
    setWindowIcon(QIcon(":/ico.png"));
    ui->mainToolBar->hide();
    ui->menuBar->hide();
    ui->statusBar->hide();

    ui->progressBar->setStyleSheet("QProgressBar{border: 1px solid gray;background: white;color:green;}");
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->hide();
    ui->progressBar->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QFont font("Microsoft YaHei");
    ui->label->setFont(font);
    ui->label->setText("请选择程序文件夹");
    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setStyleSheet("color:#6b7b89;font-size:13px;");

    ui->pushButton->setText("选择程序文件夹");
    ui->pushButton->setFont(font);
    ui->pushButton->setStyleSheet("QPushButton{border-radius:5px;border-image:null;background-color:rgb(27,188,194);color:white;font-size:13px;}"\
                                  "QPushButton:pressed{background-color:rgb(24,163,168);color:white;font-size:13px;}");
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(onOpenDirectory()));

    workthread = new WorkThread();
    workthread->setObverser(this);

    connect(workthread, SIGNAL(sigWriteFileReject(QString)), this, SLOT(onWriteFileReject(QString)));
    connect(workthread, SIGNAL(sigSetFileMD5(QString)), this, SLOT(onSetFileMD5(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpenDirectory()
{
    const QString directoryName = QFileDialog::getExistingDirectory(this);

    if(directoryName.isEmpty())
        return;

    ui->progressBar->setValue(1);
    ui->progressBar->show();
    ui->pushButton->hide();

    ui->label->setText("计算文件个数...");

    workthread->setDir(directoryName);
    workthread->start();
}

void MainWindow::onWriteFileReject(QString file){
    ui->label->setText(file + "被占用");
}

void MainWindow::onSetFileMD5(QString file){
    ui->label->setText(file);

    readCount++;
    ui->progressBar->setValue(1 + readCount * 99 / fileCount);

    if("版本文件完成" == file){
        ui->pushButton->show();
        ui->progressBar->hide();
    }
}

void MainWindow::setFileCount(int n){
    fileCount = n;
    readCount = 0;
}

