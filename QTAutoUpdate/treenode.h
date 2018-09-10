#ifndef TREENODE_H
#define TREENODE_H

#include <QList>
#include <QMap>

class treenode
{
public:
    treenode();
    void addAttri(QString, QString);
    QString getAttri(QString);
    treenode* addChild();
    treenode* getChildAt(int);
    int getChildrenCount();
    void closeChild();
    treenode* getParent();
    void setParent(treenode*);
    void setType(QString);
    QString getType();

private:
    QList<treenode*> children;
    QMap<QString, QString> attributes;
    treenode* parent;
    QString type;
};

#endif // TREENODE_H
