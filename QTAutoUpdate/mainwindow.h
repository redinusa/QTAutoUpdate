#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QDir>

class versionparser;
class WorkThread;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void onWriteFileReject(QString);
    void onWriteFileOk(QString);
    void onRetryDownload(bool);
    void onAutoUpdateFinish(bool);
    void onUserChooseUpdate();
    void callExcute();
    void replyVersionFinished(QNetworkReply *reply);
    void replyFileFinished(QNetworkReply *reply);
public:
    void TextTipWindow(QString text);
    void startSyncTask();
    void downloadFile(QString);
    void saveExecuteArg(int argv, char * argc[]);

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    QString GetServerConfig(QString setting);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* networkManager;
    versionparser* versionParser;
    QString remoteAppDir;
    WorkThread * workThread;
    QStringList args;
};

#endif // MAINWINDOW_H
