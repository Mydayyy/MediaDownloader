#ifndef TREENODE_H
#define TREENODE_H

#include <QList>
#include "link.h"

struct TreeNode {
    TreeNode();
    TreeNode(Link *link);
    TreeNode(Link *link, TreeNode *parentNode);
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

    void setLink(Link *newLink);
    Link *getLink();

    QVariant data(int column);

    TreeNode *getParentNode();

private:
    Link *link;
    TreeNode *parentNode;
    QList<TreeNode*> childNodes;
};

#endif // TREENODE_H
