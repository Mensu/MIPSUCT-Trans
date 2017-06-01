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
    using Pointer = std::unique_ptr<BallTreeNode>;

    virtual void Accept(BallTreeVisitor& v) const = 0;

    std::vector<float> center;
    double radius;

  protected:
    BallTreeNode(std::vector<float>&& center, double radius)
        : center(std::move(center)), radius(radius) {}
};

struct BallTreeBranch : BallTreeNode {
    using Pointer = std::unique_ptr<BallTreeBranch>;

    static Pointer Create(
        std::vector<float>&& center, double radius, BallTreeNode::Pointer left,
        BallTreeNode::Pointer right) {
        return std::make_unique<BallTreeBranch>(
            std::move(center), radius, std::move(left), std::move(right));
    }

    BallTreeBranch(
        std::vector<float>&& center, double radius, BallTreeNode::Pointer left,
        BallTreeNode::Pointer right)
        : BallTreeNode(std::move(center), radius),
          left(std::move(left)),
          right(std::move(right)) {}

    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }

    std::unique_ptr<BallTreeNode> left, right;
};

struct BallTreeLeaf : BallTreeNode {
    using Pointer = std::unique_ptr<BallTreeLeaf>;

    static Pointer Create(
        std::vector<float>&& center, double radius, std::vector<Rid>&& data) {
        return std::make_unique<BallTreeLeaf>(
            std::move(center), radius, std::move(data));
    }

    BallTreeLeaf(
        std::vector<float>&& center, double radius, std::vector<Rid>&& data)
        : BallTreeNode(std::move(center), radius), data(std::move(data)) {}

    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }

    std::vector<Rid> data;
};

#endif  // __BALL_TREE_NODE
