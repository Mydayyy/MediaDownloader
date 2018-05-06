#ifndef TREENODE_H
#define TREENODE_H

#include <QList>
#include <QVariant>

class MediaObject;
struct TreeNode {
    TreeNode();
    TreeNode(MediaObject *link);
    TreeNode(MediaObject *link, TreeNode *parentNode);
    ~TreeNode();

    void appendChildNode(TreeNode *child);
    void insertChildNode(TreeNode *child, int row);
    void deleteChildNode(int row);
    void removeChildNode(int row);

    int getChildNodeCount();
    TreeNode *getChildNode(int row);
    QList<TreeNode *> getChildNodes();

    int row();
    int columnCount();

    void setLink(MediaObject *newLink);
    MediaObject *getLink();

    QVariant data(int column);

    TreeNode *getParentNode();
    void setParentNode(TreeNode *parent);

    QVariant getSettingsValue(QString key);

private:
    MediaObject *link;
    TreeNode *parentNode;
    QList<TreeNode*> childNodes;
};

#endif // TREENODE_H
