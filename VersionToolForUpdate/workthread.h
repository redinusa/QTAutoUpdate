#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QThread>

class MainWindow;
class QXmlStreamWriter;
class WorkThread : public QThread
{
    Q_OBJECT
public:
    WorkThread();
    void setDir(QString);
    void setObverser(MainWindow*);
    void enumDir(QXmlStreamWriter& writer, QString _dir, QString releativeDir);

public:
    int caculateFileCount(QString dir);

protected:
    virtual void run();

signals:
    void sigWriteFileReject(QString);
    void sigSetFileMD5(QString);

private:
    QString dirname;
    MainWindow* obverser;
    int fileCount;
};

#endif // WORKTHREAD_H
