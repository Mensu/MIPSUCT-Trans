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
        BallTreeNode::Pointer right, Rid l, Rid r) {
        Rid null(0, 0, 0);
        return std::make_unique<BallTreeBranch>(
            std::move(center), radius, std::move(left), std::move(right),
            std::move(l), std::move(r), std::move(null));
    }

    static Pointer Create(
        std::vector<float>&& center, double radius, BallTreeNode::Pointer left,
        BallTreeNode::Pointer right) {
        Rid null(0, 0, 0);
        return std::make_unique<BallTreeBranch>(
            std::move(center), radius, std::move(left), std::move(right),
            null, null, null);
    }

    BallTreeBranch(
        std::vector<float>&& center, double radius, BallTreeNode::Pointer left,
        BallTreeNode::Pointer right, Rid l, Rid r, Rid id)
        : BallTreeNode(std::move(center), radius),
          left(std::move(left)),
          right(std::move(right)),
          r_left(std::move(l)), r_right(std::move(r)),
          rid(std::move(id)) {}

    // don't use
    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }

    std::unique_ptr<BallTreeNode> left, right;
    Rid r_left, r_right;
    Rid rid;
};

struct BallTreeLeaf : BallTreeNode {
    using Pointer = std::unique_ptr<BallTreeLeaf>;

    static Pointer Create(
        std::vector<float>&& center, double radius, std::vector<Rid>&& data) {
        Rid null(0, 0, 0);
        return std::make_unique<BallTreeLeaf>(
            std::move(center), radius, std::move(data), std::move(null));
    }

    BallTreeLeaf(
        std::vector<float>&& center, double radius, std::vector<Rid>&& data,
        Rid&& r)
        : BallTreeNode(std::move(center), radius), data(std::move(data)),
        rid(std::move(r)) {}

    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }

    std::vector<Rid> data;
    Rid rid;
};


#endif  // __BALL_TREE_NODE
