#include "NodeBuilder.h"



void NodeStorer::Visit(BallTreeBranch* branch) {
	branch->r_left = branch->left->rid;
	branch->r_right = branch->right->rid;
	Rid r = node_storage_->Put(*branch);
	branch->rid = r;
}

void NodeStorer::Visit(BallTreeLeaf* leaf) {
	std::vector<Rid> rids(StoreAll(leaf->raw_data));
	Rid r = node_storage_->Put(*leaf);
	leaf->rid = r;
}

std::vector<Rid> NodeStorer::StoreAll(const Records& records) {
    std::vector<Rid> ret;
    ret.reserve(records.size());
    std::transform(
        begin(records), end(records), std::back_inserter(ret),
        [this](const Record::Pointer& record) {
            return record_storage_->Put(*record);
        });
    return ret;
}



void NodeBuilder::Visit(const BallTreeBranch* branch) {
	if (branch->left) {
		v.push_back(branch->left.get());
	}
	if (branch->right) {
		v.push_back(branch->right.get());
	}
}

void NodeBuilder::Visit(const BallTreeLeaf* leaf) {
	return;
}

