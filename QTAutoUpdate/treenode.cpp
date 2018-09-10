#include "treenode.h"

treenode::treenode()
{
    parent = 0;
}

void treenode::addAttri(QString key, QString value){
    attributes.insert(key, value);
}

QString treenode::getAttri(QString key){
    QMap<QString, QString>::const_iterator it = attributes.find(key);

    if(it != attributes.end()){
        return it.value();
    }

    return "";
}

treenode* treenode::addChild(){
    treenode* child = new treenode();
    child->setParent(this);
    children.push_back(child);
    return child;
}

treenode* treenode::getChildAt(int i){
    if(children.size() <=i )
        return 0;

    return children[i];
}

int treenode::getChildrenCount(){
    return children.size();
}

void treenode::closeChild(){

}

treenode* treenode::getParent(){
    return parent;
}

void treenode::setParent(treenode* node){
    parent = node;
}

void treenode::setType(QString text){
    type = text;
}

QString treenode::getType(){
    return type;
}
