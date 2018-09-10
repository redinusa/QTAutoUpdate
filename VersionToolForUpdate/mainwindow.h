#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class WorkThread;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onOpenDirectory();
    void onWriteFileReject(QString);
    void onSetFileMD5(QString);

public:
    void setFileCount(int);

private:
    Ui::MainWindow *ui;
    WorkThread* workthread;
    int fileCount;
    int readCount;
};

#endif // MAINWINDOW_H
