#include <QCryptographicHash>
#include <QFileDialog>
#include <QDir>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QSettings>
#include "workthread.h"
#include "mainwindow.h"

WorkThread::WorkThread()
{

}

void WorkThread::setDir(QString _dir){
    dirname = _dir;
}

void WorkThread::enumDir(QXmlStreamWriter& writer, QString _dir, QString releativeDir){
    QDir dir(_dir);

    foreach (QFileInfo info, dir.entryInfoList()) {
        if(info.isDir()){
            if(info.fileName() != "." && info.fileName() != "..") // system hidden dir
            {
                writer.writeStartElement("dir");
                writer.writeAttribute("name", releativeDir + (releativeDir.isEmpty()?"":"/") + info.fileName());
                enumDir(writer, info.filePath(), releativeDir + (releativeDir.isEmpty()?"":"/") + info.fileName());
                qDebug() << info.filePath();
                writer.writeEndElement();
            }
        }
        else {
            if(releativeDir == "" && info.fileName() == "version.version"){ // do not add itself
                continue;
            }

            QFile theFile(info.filePath());
            theFile.open(QIODevice::ReadOnly);

            qint64 memlimit_32bitCPU = (qint64)1*1024*1024*1024;
            if(theFile.size() > memlimit_32bitCPU){
                qDebug() << info.fileName() << "size big than 1gb, skip";
                theFile.close();
                continue;
            }

            qDebug() << _dir + "/" + info.fileName();

            QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
            theFile.close();
            //qDebug() << ba.toHex();
            writer.writeStartElement("file");
            writer.writeAttribute("md5", ba.toHex());
            writer.writeAttribute("name", releativeDir + (releativeDir.isEmpty()?"":"/") + info.fileName());
            if(fileCount < 100){
                QThread::msleep(100); // slow down or user see nothing ~_~
            }
            writer.writeCharacters("");
            writer.writeEndElement();
            emit sigSetFileMD5("加入版本信息" + info.fileName());
        }
    }
}

int WorkThread::caculateFileCount(QString _dir){
    QDir dir(_dir);

    foreach (QFileInfo info, dir.entryInfoList())
    {
        if(info.isDir()){
            if(info.fileName() != "." && info.fileName() != ".."){
                caculateFileCount(info.filePath());
            }
        }
        else {
            fileCount++;
        }
    }

    return fileCount;
}

void WorkThread::setObverser(MainWindow* window){
    obverser = window;
}

void WorkThread::run(){
    fileCount = 0;
    fileCount = caculateFileCount(dirname);
    obverser->setFileCount(fileCount);

    /*
     * 下面这句代码,如果去掉QString::number,结果是"文件个数"这个字符串的地址被读取
     * 然后地址偏移fileCount字节,然后程序就异常了
     * 在调试这个bug的时候发现bug出现随机,debug模式没有出现过,release偶尔出现
     * 莫名其妙不合逻辑的bug,原来是个低级错误
    */
    emit sigSetFileMD5("文件个数" + QString::number(fileCount));

    // try get old version
    int oldversion = 0;
    QFile fileold(dirname + "/version.version");
    if(!fileold.open(QFile::ReadOnly)){
        oldversion = 0; // there's no old version
    }
    else {
        QXmlStreamReader reader;
        reader.setDevice(&fileold);

        while (!reader.atEnd()){
            QXmlStreamReader::TokenType nType = reader.readNext();
            if(QXmlStreamReader::StartElement == nType){
                if(reader.name() == "version"){
                    QXmlStreamAttributes attributes = reader.attributes();
                    if(attributes.hasAttribute("intversion")){
                        oldversion = attributes.value("intversion").toInt();
                        break;
                    }
                }
            }
        }
    }

    int newversion = oldversion + 1;

    // get text version set by user
    QSettings *configIniRead = new QSettings(dirname + "/versionconfig.ini", QSettings::IniFormat);
    QString textversion = configIniRead->value("info/textversion").toString();
    int force = configIniRead->value("info/forceupdate").toInt();

    QFile file(dirname + "/version.version");

    if(!file.open(QFile::WriteOnly)){
        emit sigWriteFileReject("version.version");
        return;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement("version");
    stream.writeAttribute("intversion", QString::number(newversion));
    stream.writeAttribute("textversion", textversion);
    stream.writeAttribute("forceupdate", QString::number(force));

    stream.writeTextElement("total", "");

    stream.writeStartElement("files");

    enumDir(stream, dirname, "");

    stream.writeEndElement(); // files
    stream.writeEndElement(); // version
    stream.writeEndDocument();

    emit sigSetFileMD5("版本文件完成");

    file.close();
}
