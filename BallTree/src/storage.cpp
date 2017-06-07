#include "storage.h"
constexpr std::string root_file = "/root";
constexpr std::string dimension_file = "/dimension.bin";
constexpr std::ios_base::openmode openmode = std::ios_base::trunc | std::ios_base::out | std::ios_base::binary;
NodeStorage::NodeStorage(const Path& dest_dir, int dimension = -1)
                        : m_dimension(dimension),
                        branch_storage(nullptr),
                        leaf_storage(nullptr) {
    if (m_dimension == -1) {
        std::ifstream others(dest_dir + root_file, std::ios_base::in | std::ios_base::binary);
        others.seekp(std::ios_base::beg);
        others.read(reinterpret_cast<char*>(&root), sizeof(Rid));
        others.read(reinterpret_cast<char*>(&m_dimension), sizeof(m_dimension));
    } else {
        std::ofstream others(dest_dir + root_file, std::ios_base::out | std::ios_base::binary);
        others.seekg(std::ios_base::beg, sizeof(Rid));
        others.write(reinterpret_cast<char*>(&m_dimension), sizeof(m_dimension));
    }
    size_t branch_size = Slot::GetSize(Rid::branch, dimension);
    size_t leaf_size = Slot::GetSize(Rid::leaf, dimension);
    branch_storage = std::make_unique<BranchStorage>(branch_size, "branch", dest_dir);
    leaf_storage = std::make_unique<LeafStorage>(leaf_size, "leaf", dest_dir);
}
std::unique_ptr<BallTreeNode> NodeStorage::Get(Rid rid) {
    switch (rid.type) {
        case Rid::branch:
            return std::move(branch_storage.Get<BallTreeBranch>(rid));
        case Rid::leaf:
            return std::move(leaf_storage.Get<BallTreeLeaf>(rid));
        default:
    }
    return nullptr;
}
Rid NodeStorage::Put(const BallTreeNode& node) {
    switch (rid.type) {
        case Rid::branch:
            return branch_storage.Put<BallTreeBranch>(node);
        case Rid::branch:
            return branch_storage.Put<BallTreeLeaf>(node);
        default:
    }
    return Rid(0, 0);
}

std::unique_ptr<BallTreeNode> NodeStorage::GetRoot() {
    return branch_storage.Get<BallTreeBranch>(root);
}
Rid NodeStorage::PutRoot(const BallTreeNode& node) {
    root = branch_storage.Put<BallTreeBranch>(node);
    std::ofstream others(dest_dir + root_file, std::ios_base::out | std::ios_base::binary);
    others.seekg(std::ios_base::beg);
    others.write(reinterpret_cast<char*>(root), sizeof(Rid));
    others.flush();
    return root;
}

NormalStorage::NormalStorage(const Path& dest_dir, int dimension = -1) {}
    if (dimension == -1) {
        std::ifstream others(dest_dir + root_file, std::ios_base::in | std::ios_base::binary);
        others.seekg(std::ios_base::beg);
        others.read(reinterpret_cast<char*>(&dimension), sizeof(dimension));
    } else {
        std::ofstream others(dest_dir + root_file, std::ios_base::out | std::ios_base::binary);
        others.seekp(std::ios_base::beg);
        others.write(reinterpret_cast<char*>(&dimension), sizeof(dimension));
    }
}
Rid NormalStorage::Put(const Record& record) {
    return std::move(storage.Put<Record>(record));
}
std::unique_ptr<Record> NormalStorage::Get(const Rid& rid) {
    return std::move(storage.Get<Record>(rid));
}