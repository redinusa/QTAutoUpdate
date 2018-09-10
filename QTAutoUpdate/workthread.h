#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QThread>
#include <QList>
#include <QtNetwork/QNetworkReply>
#include <QMutex>

class MainWindow;

class WorkThread : public QThread
{
    Q_OBJECT
public:
    WorkThread();
    void setObverser(MainWindow*);
    void addTask(QString, QNetworkReply *reply);

protected:
    virtual void run();

signals:
    void sigWriteFileReject(QString);
    void sigWriteFileOk(QString);

private:
    MainWindow* obverser;
    QList<QString> tasklist;
    QByteArray bytes;
    QMutex mutex;
};

#endif // WORKTHREAD_H
