#include "NodeBuilder.h"


NodeStorer::NodeStorer(NodeStorage* n_storage) : node_storage_(n_storage) {}

void NodeStorer::Visit(const BallTreeBranch* branch) {
	branch->r_left = branch->left->rid;
	branch->r_right = branch->right->rid;
	Rid r = node_storage_->Put(branch);
	leaf->rid = r;
}

void NodeStorer::Visit(const BallTreeLeaf* leaf) {
	std::vector<Rid> rids(StoreAll(records));
	Rid r = node_storage_->Put(leaf);
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
		v.push(branch->left);
	}
	if (branch->right) {
		v.push(branch->right);
	}
}

void NodeBuilder::Visit(const BallTreeLeaf* leaf) {
	return;
}

