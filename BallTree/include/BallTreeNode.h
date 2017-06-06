#ifndef __BALL_TREE_NODE_H
#define __BALL_TREE_NODE_H

#include <memory>
#include <vector>
#include "rid.h"

#include "BallTreeVisitor.h"


struct BallTreeNode {
    using Pointer = std::unique_ptr<BallTreeNode>;

    virtual void Accept(BallTreeVisitor& v) const = 0;

    std::vector<float> center;
    double radius;

    virtual ~BallTreeNode() {}

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

    // don't use
    virtual void Accept(BallTreeVisitor& v) const override {
        return;
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


struct ExBallTreeBranch : BallTreeNode {

    ExBallTreeBranch(
        std::vector<float>&& center, double radius, Rid&& left,
        Rid&& right)
        : BallTreeNode(std::move(center), radius),
          left(std::move(left)),
          right(std::move(right)) {}

    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }

    Rid left, right;
};

#endif  // __BALL_TREE_NODE