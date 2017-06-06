#include "BallTree.h"

using Records = std::vector<Record::Pointer>;


Records BallTree::ArrayToVector(int n, int d, float** data) {
    Records v;
    v.reserve(n);

    for (int i = 0; i < n; ++i) {
        v.push_back(Record::Create(
            i + 1, std::vector<float>(data[i], data[i] + d)));
    }
    return v;
}

bool BallTree::buildTree(int n, int d, float** data) {
    impl_ = std::make_unique<BallTreeImpl>(ArrayToVector(n, d, data));
    return true;
}

bool BallTree::storeTree(const char* index_path) {
    if (not impl_) {
        return false;
    }
    return impl_->StoreTree(index_path);
}

bool BallTree::restoreTree(const char* index_path) {
    impl_ = std::make_unique<BallTreeImpl>(index_path);
    return true;
}

int BallTree::mipSearch(int d, float* query) {
    if (not impl_) {
        return -1;
    }
    return impl_->Search(std::vector<float>(query, query + d)).first;
}


/**
 * Additional task (not written now)
 */

bool BallTree::insertData(int d, float* data) {
    if (not impl_) {
        return -1;
    }
    return impl_->Insert(std::vector<float>(data, data + d));
}


bool BallTree::deleteData(int d, float* data) {
    if (not impl_) {
        return false;
    }
    return impl_->Insert(std::vector<float>(data, data + d));
}


bool BallTree::buildQuadTree(int n, int d, float** data) {
    return false;
}
