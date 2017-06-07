#include "MIPSearcher.h"
#include <iostream>
void MIPSearcher::Visit(BallTreeBranch* branch) {
    auto left = node_storage_->Get(branch->r_left);
    auto right = node_storage_->Get(branch->r_right);
    double left_mip = PossibleMip(std::move(left));
    double right_mip = PossibleMip(std::move(right));
    if (left_mip > right_mip and left_mip > cur_mip_) {
        auto child_left = node_storage_->Get(branch->r_left);
        child_left->Accept(*this);
        if (right_mip > cur_mip_) {
            auto child_right = node_storage_->Get(branch->r_right);
            child_right->Accept(*this);
        }
    } else if (right_mip >= left_mip and right_mip > cur_mip_) {
        auto child_right = node_storage_->Get(branch->r_right);
        child_right->Accept(*this);
        if (left_mip > cur_mip_) {
            auto child_left = node_storage_->Get(branch->r_left);
            child_left->Accept(*this);
        }
    } 
}
void MIPSearcher::Visit(BallTreeLeaf* leaf) {
    for (const auto& rid : leaf->data) {
        auto record = record_storage_->Get(rid);
        double innerproduct = InnerProduct(needle, record->data);
        if (innerproduct > cur_mip_) {
            cur_mip_ = innerproduct;
            cur_max_idx_ = record->index;
        }
    }
}

double MIPSearcher::PossibleMip(const std::unique_ptr<BallTreeNode> node) {
    return InnerProduct(needle, node->center) + node->radius * needle_norm;
}

