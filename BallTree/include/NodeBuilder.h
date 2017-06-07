#ifndef __NODE_BUILDER_H
#define __NODE_BUILDER_H

#include <vector>
#include "storage.h"
#include "BallTreeNode.h"




class NodeStorer : public BallTreeVisitor {
  public:
    NodeStorer(NodeStorage* n_storage) : node_storage_(n_storage) {}

    virtual void Visit(const BallTreeBranch* branch);

    virtual void Visit(const BallTreeLeaf* leaf);


  private:
    NodeStorage* node_storage_;
};

class NodeBuilder : public BallTreeVisitor {
  public:
    NodeBuilder() = default;

    virtual void Visit(const BallTreeBranch* branch);

    virtual void Visit(const BallTreeLeaf* leaf);

    std::vector<BallTreeNode*> Get() {
    	return std::move(v);
    }

  private:
    std::vector<BallTreeNode*> v;
};

#endif