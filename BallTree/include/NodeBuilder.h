#ifndef __NODE_BUILDER_H
#define __NODE_BUILDER_H

#include <vector>
#include <algorithm>
#include "storage.h"
#include "rid.h"
#include "record.h"
#include "BallTreeNode.h"




class NodeStorer : public BallTreeVisitor {
	using Records = std::vector<Record::Pointer>;
  public:
    NodeStorer(NodeStorage* n_storage, RecordStorage* r_storage)
      : node_storage_(n_storage), record_storage_(r_storage) {}

    virtual void Visit(const BallTreeBranch* branch);

    virtual void Visit(const BallTreeLeaf* leaf);

    std::vector<Rid> StoreAll(const Records& records);


  private:
    NodeStorage* node_storage_;
    RecordStorage* record_storage_;
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