#include "MIPSearcher.h"

void MIPSearcher::Visit(const ExBallTreeBranch* branch) {
    double left_mip(PossibleMip(node_storage_->Get(branch->left))),
        right_mip(PossibleMip(node_storage_->Get(branch->right)));
    if (left_mip > right_mip and left_mip > cur_mip_) {
        node_storage_->Get(branch->left)->Accept(*this);
        if (right_mip > cur_mip_) {
            node_storage_->Get(branch->right)->Accept(*this);
        }
    } else if (right_mip >= left_mip and right_mip > cur_mip_) {
        node_storage_->Get(branch->right)->Accept(*this);
        if (left_mip > cur_mip_) {
            node_storage_->Get(branch->left)->Accept(*this);
        }
    } 
}
void MIPSearcher::Visit(const BallTreeLeaf* leaf) {
    for (const auto& rid : leaf->data) {
        auto record = record_storage_->Get(rid);
        double innerproduct = InnerProduct(needle, record->data);
        if (innerproduct > cur_mip_) {
            cur_mip_ = innerproduct;
            cur_max_idx_ = record->index;
        }
    }
}

double MIPSearcher::PossibleMip(const BallTreeNode* node) {
    return InnerProduct(needle, node->center) + node->radius * needle_norm;
}
