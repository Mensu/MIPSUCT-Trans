#ifndef __BALL_TREE_NODE
#define __BALL_TREE_NODE

#include <memory>
#include <vector>
#include "rid.h"

struct BallTreeBranch;
struct BallTreeLeaf;

/**
 * visitor pattern for multiple dispatch
 */
class BallTreeVisitor {
  public:
    virtual void Visit(const BallTreeBranch*) = 0;
    virtual void Visit(const BallTreeLeaf*) = 0;
};

struct BallTreeNode {
    virtual void Accept(BallTreeVisitor& v) const = 0;
};

struct BallTreeBranch : BallTreeNode {
    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }

    std::unique_ptr<BallTreeNode> left, right;
    std::vector<float> center;
    double radius;
};

struct BallTreeLeaf : BallTreeBranch {
    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }
    std::vector<Rid> data;
};

#endif  // __BALL_TREE_NODE
