#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "versionparser.h"
#include "workthread.h"
#include <QSettings>
#include <QProcess>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(400, 300);
    setWindowFlags(windowFlags()&~ (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint));

    setWindowIcon(QIcon(":/ico.png"));
    ui->mainToolBar->hide();
    ui->menuBar->hide();
    ui->statusBar->hide();

    QFont font("Microsoft YaHei");
    ui->label->setFont(font);
    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setStyleSheet("color:rgb(122,151,22);font-size:13px;");
    ui->pushButton->setFont(font);
    ui->pushButton->setStyleSheet("border-radius:5px;color:white;background-color:#4CAF50;font-size:13px;");
    ui->pushButton->hide();

    versionParser = new versionparser();
    versionParser->setObverser(this);

    workThread = new WorkThread();
    workThread->setObverser(this);
    workThread->start();

    connect(workThread, SIGNAL(sigWriteFileReject(QString)), this, SLOT(onWriteFileReject(QString)));
    connect(workThread, SIGNAL(sigWriteFileOk(QString)), this, SLOT(onWriteFileOk(QString)));
    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(onRetryDownload(bool)));

    networkManager = new QNetworkAccessManager();

    QString config = GetServerConfig("versionconfig.ini");

    if(config.isEmpty()){
        TextTipWindow("读取config.ini失败, 无法确定最新版本");
        onAutoUpdateFinish(false);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::GetServerConfig(QString setting){
    QString appDir = QApplication::applicationDirPath();

    QSettings *configIniRead = new QSettings(appDir + "/" + setting, QSettings::IniFormat);
    QString versionUrl = configIniRead->value("info/version").toString();

    remoteAppDir = configIniRead->value("info/romoteappdir").toString();

    delete configIniRead;

    if(versionUrl.isEmpty())
        return versionUrl;

    QNetworkRequest request;
    request.setUrl(QUrl(versionUrl));

    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyVersionFinished(QNetworkReply *)));

    TextTipWindow("获取版本信息...");
    networkManager->get(request);

    return versionUrl;
}

void MainWindow::onWriteFileReject(QString route){
    int first = route.lastIndexOf ("/");
    QString name = route.right(route.length () - first - 1);

    TextTipWindow("文件" + name + "被其他程序占用, \n请结束对应程序后重试");
    ui->pushButton->setText("重试");
    ui->pushButton->show();
}

void MainWindow::onWriteFileOk(QString){
    ui->pushButton->hide();
    versionParser->exit(0);
}

void MainWindow::onRetryDownload(bool){
    QString route = versionParser->getCurrentdownFileRoute();
    QString appDir = QApplication::applicationDirPath();

    ui->pushButton->hide();
    TextTipWindow("更新文件" + route);

    workThread->addTask(appDir +"/" + route, 0);
}

void MainWindow::replyVersionFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError)
    {
        QString appDir = QApplication::applicationDirPath();

        QByteArray bytes = reply->readAll();

        // 缓存到本地
        QFile file(appDir +"/serverversion.version");
        if (file.open(QIODevice::WriteOnly))
            file.write(bytes);

        file.close();

        bool ret = versionParser->parseLocalVersion(appDir +"/version.version");
        bool ret1 = versionParser->parseServerVersion(appDir +"/serverversion.version");

        if(!ret || !ret1){
            TextTipWindow("版本文件解析失败, 文件可能已经损坏, \n请到官方网站下载最新版本程序");
            onAutoUpdateFinish(false);
        }
        else {
            disconnect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyVersionFinished(QNetworkReply *)));
            versionParser->start();
            connect(versionParser, SIGNAL(sigAutoUpdateFinish(bool)), this, SLOT(onAutoUpdateFinish(bool)));
            connect(versionParser, SIGNAL(sigUserChooseUpdateUI()), this, SLOT(onUserChooseUpdate()));
        }
    }
    else {
        TextTipWindow("无法连接到服务器");
        onAutoUpdateFinish(false);
    }
}

void MainWindow::replyFileFinished(QNetworkReply *reply) {
    QString route = versionParser->getCurrentdownFileRoute();
    QString appDir = QApplication::applicationDirPath();

    if (reply->error() == QNetworkReply::NoError) {
        TextTipWindow("更新文件" + route);

        // 发现升级程序本身需要更新,缓存新的升级程序,等待旧的升级程序更新
        if(route.contains("QTAutoUpdate"))
            route = "qtautoupdate_tmp.tmp";

        workThread->addTask(appDir +"/" + route, reply);
    }
    else {
        TextTipWindow("更新文件" + route + "失败");
        onAutoUpdateFinish(false);
    }
}

void MainWindow::TextTipWindow(QString text){
    QThread::msleep(100);
    ui->label->setText(text);
}

void MainWindow::startSyncTask() {
    // sig send by parser thread while slot called on main thread
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFileFinished(QNetworkReply *)));
    versionParser->startSyncVersion();
}

void MainWindow::onAutoUpdateFinish(bool ok){
    if(ok){ // quit and start the Excute
        callExcute();
        exit(0);
    }
    else { // show fail
        ui->pushButton->setText("确定");
        //子线程不能直接更新ui线程,
        //所以这个函数是一个槽函数, 被子线程信号激发,
        //但是在主线程调用
        ui->pushButton->show();
        connect(ui->pushButton, SIGNAL(clicked()), qApp, SLOT(quit()));
    }
}

void MainWindow::onUserChooseUpdate(){
    QMessageBox box(this);
    box.setText("是否升级到新版本?");
    box.setWindowTitle("发现新版本");
    box.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    box.setButtonText(QMessageBox::Yes, "立即升级");
    box.setButtonText(QMessageBox::No, "残忍跳过");

    int a = box.exec();

    if(QMessageBox::Yes == a)
    {
        versionParser->exit(0);
    }
    else {
        versionParser->exit(1);
        callExcute();
        exit(0);
    }
}

void MainWindow::callExcute(){
    QString appDir = QApplication::applicationDirPath();
    QSettings *configIniRead = new QSettings(appDir + "/versionconfig.ini", QSettings::IniFormat);
    QString app = configIniRead->value("info/app").toString();

    QProcess* process = new QProcess();
    if(!args.isEmpty())
        process->setArguments(args);
    process->start(appDir + "/" + app);

    QProcess* process1 = new QProcess();
    // 执行selfupdate,替换updater
    // (所以这个升级程序生气起来连自己都干掉~_~)
    // 希望updater能在1s内退出
    process1->start(appDir + "/selfupdate");
}

void MainWindow::downloadFile(QString file){
    QNetworkRequest request;
    request.setUrl(QUrl(remoteAppDir + "/" + file));

    TextTipWindow("下载" + file + "...");
    networkManager->get(request);
}

void MainWindow::saveExecuteArg(int argv, char * argc[]){
    for(int i=1; i<argv; i++){
        args.push_back(argc[i]);
    }
}

void MainWindow::closeEvent(QCloseEvent *event){
    event->ignore();
}
