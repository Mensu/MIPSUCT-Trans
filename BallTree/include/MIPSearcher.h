#ifndef __MIP_SEARCHER_H
#define __MIP_SEARCHER_H

#include <vector>
#include "storage.h"
#include "BallTreeNode.h"




class MIPSearcher : public BallTreeVisitor {
  public:
    MIPSearcher(const std::vector<float>& v, RecordStorage* storage)
        : needle(v), needle_norm(Norm(needle)), storage_(storage) {}

    virtual void Visit(const BallTreeBranch* branch);

    virtual void Visit(const BallTreeLeaf* leaf);

    int ResultIndex() const {
        return cur_max_idx_;
    }
    double ResultMIP() const {
        return cur_mip_;
    }

  private:
    double PossibleMip(const BallTreeNode* node) {
        return InnerProduct(needle, node->center) + node->radius * needle_norm;
    }

    const std::vector<float>& needle;
    const double needle_norm;
    int cur_max_idx_ = -1;
    double cur_mip_ = 0;
    RecordStorage* storage_;
};

#endif