#include "treenode.h"
#include "tablemodel.h"

TreeNode::TreeNode()
    : link(nullptr),
      parentNode(nullptr)
{
}

TreeNode::TreeNode(MediaObject *link)
    : link(link),
      parentNode(nullptr)
{
}

TreeNode::TreeNode(MediaObject *link, TreeNode *parentNode)
    : link(link),
      parentNode(parentNode)
{
}

TreeNode::~TreeNode()
{
    if(link)
    {
        delete link;
    }
    qDeleteAll(childNodes);
}

void TreeNode::appendChildNode(TreeNode *child)
{
    childNodes.append(child);
    child->setParentNode(this);
}

void TreeNode::insertChildNode(TreeNode *child, int row)
{
    if(row == -1)
    {
        appendChildNode(child);
        return;
    }
    childNodes.insert(row, child);
    child->setParentNode(this);
}

void TreeNode::deleteChildNode(int row)
{
    TreeNode *toDelete = this->getChildNode(row);
    this->childNodes.removeAt(row);
    delete toDelete;
}

void TreeNode::removeChildNode(int row)
{
    TreeNode *toRemove = this->getChildNode(row);
    toRemove->setParentNode(nullptr);
    this->childNodes.removeAt(row);
}

TreeNode *TreeNode::getChildNode(int row)
{
    return childNodes.value(row);
}

QList<TreeNode *> TreeNode::getChildNodes()
{
    return childNodes;
}

int TreeNode::getChildNodeCount()
{
    return childNodes.count();
}

void TreeNode::setLink(MediaObject *newLink)
{
    this->link = newLink;
}

MediaObject *TreeNode::getLink()
{
    return this->link;
}

QVariant TreeNode::data(int column)
{
    return this->link->getData(TableModel::mapColumnToData(column));
}

int TreeNode::row()
{
    if(parentNode)
    {
        return parentNode->childNodes.indexOf(this);
    }
    return 0;
}

int TreeNode::columnCount()
{
    return MediaObject::DISPLAY_MAX_PROPERTIES;
}

TreeNode *TreeNode::getParentNode()
{
    return this->parentNode;
}

void TreeNode::setParentNode(TreeNode *parent)
{
    this->parentNode = parent;
}

QVariant TreeNode::getSettingsValue(QString key)
{
    if(!this->parentNode) {
        return Settings::getInstance().get(key);
    }
    Settings *settings = this->getLink()->getSettings();
    if(!settings->hasKey(key)) {
        return this->parentNode->getSettingsValue(key);
    }
    return settings->get(key);
}
