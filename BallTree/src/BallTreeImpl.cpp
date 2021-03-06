#include "BallTreeImpl.h"

using Records = std::vector<Record::Pointer>;

/**
 * build the balltree from index file
 */
BallTreeImpl::BallTreeImpl(Path& index_path):dim(-1) {
    if (not record_storage_) {
        record_storage_ = storage_factory::GetRecordStorage(index_path, dim);
        node_storage_ = storage_factory::GetNodeStorage(index_path, dim);
    }
    root_ = node_storage_->GetRoot();
}

/**
 * build the balltree from plain index and vector data
 */
BallTreeImpl::BallTreeImpl(Records&& records)
    :
#ifdef BALLTREE_TESTING_ALGORITHM
      record_storage_(storage_factory::GetSimpleStorage()),
#endif
      root_(BuildTree(std::move(records))) {
}

/**
 * functions for calculations
 */
std::vector<float> BallTreeImpl::CalculateCenter(const Records& records) {
    assert(records.size() > 0);
    std::vector<float> center(records.front()->Size(), 0);
    for (auto& record : records) {
        Combine(center, record->data, std::plus<float>());
    }
    auto s = records.size();
    ApplyElementwise(center, [s](float x) { return x / s; });
    return center;
}

double BallTreeImpl::CalculateRadius(
    const Records& records, const std::vector<float>& center) {
    return std::accumulate(
        begin(records), end(records), 0.0,
        [&center](double acc, const Record::Pointer& record) {
            return std::max(Distance(center, record->data), acc);
        });
}

Record* BallTreeImpl::ChooseFarthest(const Records& records, Record* pivot) {
    double max_distance = 0;
    Record* result = pivot;
    for (auto& record : records) {
        double new_distance = Distance(record->data, pivot->data);
        if (new_distance > max_distance) {
            max_distance = new_distance;
            result = record.get();
        }
    }
    return result;
}

std::pair<Record*, Record*> BallTreeImpl::PickPivots(const Records& records) {
    assert(records.size() >= 2);
    auto& arbitrary(records.front());
    Record* a = ChooseFarthest(records, arbitrary.get());
    Record* b = ChooseFarthest(records, a);
    return {a, b};
}

std::pair<Records, Records> BallTreeImpl::SplitRecord(
    Records&& records, Record* a, Record* b) {
    auto mid = std::partition(
        begin(records), end(records),
        [a, b](const Record::Pointer& record) {
            return Distance(record->data, a->data) <
                   Distance(record->data, b->data);
        });
    return std::make_pair(
        Records(
            std::make_move_iterator(begin(records)),
            std::make_move_iterator(mid)),
        Records(
            std::make_move_iterator(mid),
            std::make_move_iterator(end(records))));
}

std::pair<Records, Records> BallTreeImpl::SplitRecord(Records&& records) {
    Record *a, *b;
    std::tie(a, b) = PickPivots(records);
    return SplitRecord(std::move(records), a, b);
}

/**
 *  Functions for building Ball Tree Node 
 */

BallTreeLeaf::Pointer BallTreeImpl::BuildTreeLeaf(Records& records) {
    std::vector<float> center(CalculateCenter(records));
    double radius(CalculateRadius(records, center));
    return BallTreeLeaf::Create(std::move(center), radius, std::move(records));
}


BallTreeBranch::Pointer BallTreeImpl::BuildTreeBranch(Records&& data) {
    std::vector<float> center(CalculateCenter(data));
    double radius(CalculateRadius(data, center));
    std::pair<Records, Records> split_result(SplitRecord(std::move(data)));
    return BallTreeBranch::Create(
        std::move(center), radius, BuildTree(std::move(split_result.first)),
        BuildTree(std::move(split_result.second)));
}

BallTreeNode::Pointer BallTreeImpl::BuildTree(Records&& records) {
    if (records.size() <= N0) {
        return BuildTreeLeaf(records);
    }
    return BuildTreeBranch(std::move(records));
}

std::vector<Rid> BallTreeImpl::StoreAll(const Records& records) {
    std::vector<Rid> ret;
    ret.reserve(records.size());
    std::transform(
        begin(records), end(records), std::back_inserter(ret),
        [this](const Record::Pointer& record) {
            return record_storage_->Put(*record);
        });
    return ret;
}



/**
 * store the balltree to an index file
 */
bool BallTreeImpl::StoreTree(Path& index_path) {
    if (not record_storage_) {
        record_storage_ = storage_factory::GetRecordStorage(index_path, dim);
        node_storage_ = storage_factory::GetNodeStorage(index_path, dim);
    }

    NodeStorer visitor(node_storage_.get(), record_storage_.get());
    root_->Accept(visitor);

    node_storage_->PutRoot(*root_.get());

    root_ = nullptr;
    record_storage_ = nullptr;
    node_storage_ = nullptr;
    return true;
}

/**
 * returns the index of the vector with the maximum inner product with the
 * vector given
 */
std::pair<int, double> BallTreeImpl::Search(const std::vector<float>& v) {
    if (not root_) {
        assert(false && "root is nullptr!");
        return {-1, 0};
    }
    MIPSearcher visitor(v, record_storage_.get(), node_storage_.get());
    root_->Accept(visitor);
    return {visitor.ResultIndex(), visitor.ResultMIP()};
}



/**
 * insert given vector to the balltree
 */
bool BallTreeImpl::Insert(const std::vector<float>& v) {
    return false;
}

/**
 * delete given vector from the balltree
 */
bool BallTreeImpl::Delete(const std::vector<float>& v) {
    return false;
}


bool BallTreeImpl::SetDimension(int d) {
    dim = d;
}