#include "storage.h"

NodeStorage::NodeStorage(const Path& dest_dir, int dimension = -1) {
    
}
std::unique_ptr<BallTreeNode> NodeStorage::Get(Rid rid) {

}
Rid NodeStorage::Put(const BallTreeNode& node) {

}

std::unique_ptr<BallTreeNode> NodeStorage::GetRoot() {

}
Rid NodeStorage::PutRoot(const BallTreeNode& node) {

}

NormalStorage::NormalStorage(const Path& dest_dir, int dimension) {

}
Rid NormalStorage::Put(const Record& record) {

}
std::unique_ptr<Record> NormalStorage::Get(const Rid& rid) {
    
}