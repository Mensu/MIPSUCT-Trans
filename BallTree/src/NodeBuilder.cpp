#include "NodeBuilder.h"



void NodeStorer::Visit(BallTreeBranch* branch) {
	branch->r_left = branch->left->rid;
	branch->r_right = branch->right->rid;
	Rid r = node_storage_->Put(*branch);
	branch->rid = r;
	// auto before_cast = node_storage_->Get(r);
	// auto ptr = dynamic_cast<BallTreeBranch *>(before_cast.get());
	// assert(ptr != nullptr);
	// assert(*ptr == *branch);
}

void NodeStorer::Visit(BallTreeLeaf* leaf) {
	leaf->data = StoreAll(leaf->raw_data);
	Rid r = node_storage_->Put(*leaf);
	leaf->rid = r;
	// auto before_cast = node_storage_->Get(r);
	// auto ptr = dynamic_cast<BallTreeLeaf *>(before_cast.get());
	// assert(ptr != nullptr);
	// assert(*ptr == *leaf);
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



void NodeBuilder::Visit(BallTreeBranch* branch) {
	if (branch->left) {
		v.push_back(branch->left.get());
	}
	if (branch->right) {
		v.push_back(branch->right.get());
	}
}

void NodeBuilder::Visit(BallTreeLeaf* leaf) {
	return;
}

