#include "workthread.h"
#include "mainwindow.h"

WorkThread::WorkThread()
{

}

void WorkThread::setObverser(MainWindow* watcher){
    obverser = watcher;
}

void WorkThread::addTask(QString task, QNetworkReply *reply){
    mutex.lock();

    tasklist.push_back(task);

    if(reply){
        bytes = reply->readAll();
    }
    else{ // bytes already set by latstime call 'addtask', this is a retry call by mainwindow

    }

    mutex.unlock();
}

void WorkThread::run(){
    while(true){
        QThread::msleep(100);
        if(!tasklist.isEmpty()){
            mutex.lock();
            QByteArray _bytes;
            // 不调用这句代码造成_bytes没有初始化,指向一个随机地址,执行mem copy,
            // 最终造成heap occupied error
            _bytes.resize(bytes.size());

            memcpy(_bytes.data(), bytes.data(), bytes.size());
            QString task = tasklist.at(0);
            tasklist.pop_front();
            mutex.unlock();

            QFile file(task);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(_bytes);
                emit sigWriteFileOk(task);
            }
            else {
                emit sigWriteFileReject(task);
            }

            file.close();

        }
    }
}
