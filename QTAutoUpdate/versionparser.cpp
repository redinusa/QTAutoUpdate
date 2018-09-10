#include "versionparser.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

versionparser::versionparser()
{
    treeNodeLocal = new treenode();
    treeNodeServer = new treenode();
}

bool versionparser::parseLocalVersion(QString fileName){
    QFile file(fileName);

    if(!file.open(QFile::ReadOnly))
            return false;

    QXmlStreamReader reader;
    reader.setDevice(&file);

    treenode* current = treeNodeLocal;

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType nType = reader.readNext();

        if(!current)
            break;

        switch (nType) {
        case QXmlStreamReader::StartElement:
            {
                treenode* child = current->addChild();
                current = child;

                QXmlStreamAttributes attributes = reader.attributes();
                for(int i = 0; i < attributes.size(); i++){
                    QXmlStreamAttribute attr = attributes[i];
                    current->addAttri(attr.name().toString(), attr.value().toString());
                }

                current->setType(reader.name().toString());
            }
            break;
        case QXmlStreamReader::EndElement:
            current->closeChild();
            current = current->getParent();
            break;
        default:
            break;
        }
    }

    if(reader.hasError())
        return false;

    return true;
}

bool versionparser::parseServerVersion(QString fileName){
    QFile file(fileName);

    if(!file.open(QFile::ReadOnly))
            return false;

    QXmlStreamReader reader;
    reader.setDevice(&file);

    treenode* current = treeNodeServer;

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType nType = reader.readNext();

        if(!current)
            break;

        switch (nType) {
        case QXmlStreamReader::StartElement:
            {
                treenode* child = current->addChild();
                current = child;

                QXmlStreamAttributes attributes = reader.attributes();
                for(int i = 0; i < attributes.size(); i++){
                    QXmlStreamAttribute attr = attributes[i];
                    current->addAttri(attr.name().toString(), attr.value().toString());
                }

                current->setType(reader.name().toString());
            }
            break;
        case QXmlStreamReader::EndElement:
            current->closeChild();
            current = current->getParent();
            break;
        default:
            break;
        }
    }

    if(reader.hasError())
        return false;

    return true;
}

void versionparser::startSyncVersion(){
    if(treeNodeServer->getChildrenCount() > 0 && treeNodeLocal->getChildrenCount() > 0){
        int iVersionLocal = treeNodeLocal->getChildAt(0)->getAttri("intversion").toInt();
        int iVersionServer = treeNodeServer->getChildAt(0)->getAttri("intversion").toInt();
        int force = treeNodeServer->getChildAt(0)->getAttri("forceupdate").toInt();

        if(iVersionServer != iVersionLocal){ // if server is new
            QString serverVersion = treeNodeServer->getChildAt(0)->getAttri("textversion");
            obverser->TextTipWindow("程序最新版本" + serverVersion + ", 准备升级");

            treenode* fileLocal = getFilesNode(treeNodeLocal->getChildAt(0));
            treenode* fileServer = getFilesNode(treeNodeServer->getChildAt(0));

            if(!force){
                sigUserChooseUpdateUI();
                int a = exec();
                if(0 != a ){ // user choose not update
                    this->exit(0);
                    return;
                }
            }

            if(fileLocal && fileServer) {
                sync(fileServer, fileLocal);
            }
            else {
                obverser->TextTipWindow("版本文件错误, 无法自动升级");
                sigAutoUpdateFinish(false);
            }
        }
        else {
            obverser->TextTipWindow("当前已经是最新版本, 无需升级");
            sigAutoUpdateFinish(true);
        }
    }
}

void versionparser::setObverser(MainWindow* watcher){
    obverser = watcher;
}

void versionparser::run() {
    obverser->startSyncTask();
}

treenode* versionparser::getFilesNode(treenode* node){
    for(int i=0; i<node->getChildrenCount(); i++) {
        if(QString::compare(node->getChildAt(i)->getType(), "files") == 0){
            return node->getChildAt(i);
        }
    }

    return 0;
}

QString versionparser::getCurrentdownFileRoute() {
    return currentDownfileName;
}

void versionparser::sync(treenode* from, treenode* to){
    // start to list 'from' node files and dirs
    for(int i=0; i<from->getChildrenCount(); i++) {
        treenode* child = from->getChildAt(i);

        if(QString::compare(child->getType(), "file") == 0) { // file
            QString name = child->getAttri("name");
            //int fromVersion = child->getAttri("intversion").toInt(); // user int version to compare
            QString md5 = child->getAttri("md5");
            treenode* node = findNodeByName(to, name);

            //if(!node || node->getAttri("intversion").toInt() < fromVersion){ // user int version to compare, file should be download
            if(!node || node->getAttri("md5") != md5){
                QThread::msleep(100);
                currentDownfileName = name;
                obverser->downloadFile(name);
                exec();
            }
            else { // file is newest, do nothing

            }
            qDebug() << name;
        }
        else if(QString::compare(child->getType(), "dir") == 0) { // dir
            QString name = child->getAttri("name");
            //int fromVersion = child->getAttri("intversion").toInt(); // user int version to compare
            treenode* node = findNodeByName(to, name);

            //if (!node || node->getAttri("intversion").toInt() < fromVersion) { // user int version to compare, should update dir
            if(true) {// always check dir
                if(!node) { // create dir
                    QString appDir = QApplication::applicationDirPath();
                    QDir dir;
                    if(!dir.exists(appDir + "/" + name)){
                        dir.mkpath(appDir + "/" + name);
                    }
                }

                // update files in dir
                sync(child, node);
            }
            else { // dir is newest, do nothing

            }
            qDebug() << child->getAttri("name");
        }
    }

    if(from == getFilesNode(treeNodeServer->getChildAt(0))){ // root node sync over
        obverser->TextTipWindow("自动更新完成");
        QString appDir = QApplication::applicationDirPath();
        QFile file(appDir + "/version.version");
        file.remove();

        QFile file1(appDir + "/serverversion.version");
        file1.rename(appDir + "/version.version");

        sigAutoUpdateFinish(true);
    }
}

treenode* versionparser::findNodeByName(treenode* parent, QString name){
    if(!parent)
        return 0;

    for(int i=0; i<parent->getChildrenCount(); i++) {
        treenode* child = parent->getChildAt(i);

        if(QString::compare(child->getAttri("name"), name) == 0) { // file
            return child;
        }
    }

    return 0;
}
