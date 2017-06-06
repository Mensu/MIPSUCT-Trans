#include "MIPSearcher.h"

void MIPSearcher::Visit(const BallTreeBranch* branch) {
    double left_mip(PossibleMip(branch->left.get())),
        right_mip(PossibleMip(branch->right.get()));
    if (left_mip > right_mip and left_mip > cur_mip_) {
        branch->left->Accept(*this);
        if (right_mip > cur_mip_) {
            branch->right->Accept(*this);
        }
    } else if (right_mip >= left_mip and right_mip > cur_mip_) {
        branch->right->Accept(*this);
        if (left_mip > cur_mip_) {
            branch->left->Accept(*this);
        }
    } 
}
void MIPSearcher::Visit(const BallTreeLeaf* leaf) {
    for (const auto& rid : leaf->data) {
        auto record = storage_->Get(rid);
        double innerproduct = InnerProduct(needle, record->data);
        if (innerproduct > cur_mip_) {
            cur_mip_ = innerproduct;
            cur_max_idx_ = record->index;
        }
    }
}
