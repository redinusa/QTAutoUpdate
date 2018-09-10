#ifndef VERSIONPARSER_H
#define VERSIONPARSER_H

#include <QThread>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttribute>
#include "treenode.h"

class MainWindow;
class versionparser : public QThread
{
    Q_OBJECT
public:
    versionparser();
    bool parseLocalVersion(QString);
    bool parseServerVersion(QString);
    void startSyncVersion();
    void setObverser(MainWindow*);
    treenode* getFilesNode(treenode*);
    QString getCurrentdownFileRoute();

protected:
    virtual void run();

signals:
    void sigAutoUpdateFinish(bool);
    void sigUserChooseUpdateUI();

private:
    void sync(treenode*, treenode*);
    treenode* findNodeByName(treenode*, QString);

private:
    MainWindow* obverser;
    treenode* treeNodeLocal;
    treenode* treeNodeServer;
    QString currentDownfileName;
};

#endif // VERSIONPARSER_H
