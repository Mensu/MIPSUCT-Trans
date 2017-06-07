#include "NodeBuilder.h"


NodeStorer::NodeStorer(NodeStorage* n_storage) : node_storage_(n_storage) {}

void NodeStorer::Visit(const BallTreeBranch* branch) {
	branch->r_left = branch->left->rid;
	branch->r_right = branch->right->rid;
	Rid r = node_storage_->Put(branch);
	leaf->rid = r;
}

void NodeStorer::Visit(const BallTreeLeaf* leaf) {
	Rid r = node_storage_->Put(leaf);
	leaf->rid = r;
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


