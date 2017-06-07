#ifndef __BALL_TREE_NODE_H
#define __BALL_TREE_NODE_H

#include <memory>
#include <vector>
#include "rid.h"
#include "record.h"
#include "BallTreeVisitor.h"


struct BallTreeNode {
    using Pointer = std::unique_ptr<BallTreeNode>;

    virtual void Accept(BallTreeVisitor& v) const = 0;

    std::vector<float> center;
    double radius;
    Rid rid;

    virtual ~BallTreeNode() {}

  protected:
    BallTreeNode(std::vector<float>&& center, double radius, Rid r)
        : center(std::move(center)), radius(radius), rid(std::move(r)) {}
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
        : BallTreeNode(std::move(center), radius, std::move(id)),
          left(std::move(left)),
          right(std::move(right)),
          r_left(std::move(l)), r_right(std::move(r)) {}

    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }

    std::unique_ptr<BallTreeNode> left, right;
    Rid r_left, r_right;
};

struct BallTreeLeaf : BallTreeNode {
    using Pointer = std::unique_ptr<BallTreeLeaf>;
    using Records = std::vector<Record::Pointer>;

    static Pointer Create(
        std::vector<float>&& center, double radius, std::vector<Rid>&& d) {
        Rid null(0, 0, 0);
        Records nullrecord;
        return std::make_unique<BallTreeLeaf>(
            std::move(center), radius, std::move(d), std::move(null),
            std::move(nullrecord));
    }

    static Pointer Create(
        std::vector<float>&& center, double radius, Records records) {
        Rid null(0, 0, 0);
        std::vector<Rid> d;
        return std::make_unique<BallTreeLeaf>(
            std::move(center), radius, std::move(d), std::move(null),
            std::move(records));
    }

    BallTreeLeaf(
        std::vector<float>&& center, double radius, std::vector<Rid>&& d,
        Rid&& r, Records records)
        : BallTreeNode(std::move(center), radius, std::move(r)),
        data(std::move(d)), raw_data(std::move(records)) {}

    virtual void Accept(BallTreeVisitor& v) const override {
        v.Visit(this);
    }

    std::vector<Rid> data;
    std::vector<Record::Pointer> raw_data;
};


#endif  // __BALL_TREE_NODE
