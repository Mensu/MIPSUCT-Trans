#ifndef __BALL_TREE_H
#define __BALL_TREE_H

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "Utility.h"
#include "balltree-node.h"
#include "record.h"
#include "storage.h"

constexpr int N0 = 20;

namespace detail {
class MIPSearcher : public BallTreeVisitor {
  public:
    MIPSearcher(const std::vector<float>& v, RecordStorage* storage)
        : needle(v), needle_norm(Norm(needle)), storage_(storage) {}

    virtual void Visit(const BallTreeBranch* branch) {
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
    virtual void Visit(const BallTreeLeaf* leaf) {
        for (const auto& rid : leaf->data) {
            auto record = storage_->Get(rid);
            double innerproduct = InnerProduct(needle, record->data);
            if (innerproduct > cur_mip_) {
                cur_mip_ = innerproduct;
                cur_max_idx_ = record->index;
            }
        }
    }

    int ResultIndex() const {
        return cur_max_idx_;
    }
    double ResultMIP() const {
        return cur_mip_;
    }

  private:
    double PossibleMip(const BallTreeNode* node) {
        return InnerProduct(needle, node->center) + node->radius * needle_norm;
    }

    const std::vector<float>& needle;
    const double needle_norm;
    int cur_max_idx_ = -1;
    double cur_mip_ = 0;
    RecordStorage* storage_;
};
}

using Records = std::vector<Record::Pointer>;

class BallTreeImpl {
  public:
    std::vector<Rid> StoreAll(const Records& records) {
        std::vector<Rid> ret;
        ret.reserve(records.size());
        std::transform(
            begin(records), end(records), std::back_inserter(ret),
            [this](const Record::Pointer& record) {
                return record_storage_->Put(*record);
            });
        return ret;
    }

    BallTreeLeaf::Pointer BuildTreeLeaf(const Records& records) {
        std::vector<Rid> rids(StoreAll(records));
        std::vector<float> center(CalculateCenter(records));
        double radius(CalculateRadius(records, center));
        return BallTreeLeaf::Create(std::move(center), radius, std::move(rids));
    }

    static std::vector<float> CalculateCenter(const Records& records) {
        assert(records.size() > 0);
        std::vector<float> center(records.front()->Size(), 0);
        for (auto& record : records) {
            Combine(center, record->data, std::plus<float>());
        }
        auto s = records.size();
        ApplyElementwise(center, [s](float x) { return x / s; });
        return center;
    }

    static double CalculateRadius(
        const Records& records, const std::vector<float>& center) {
        return std::accumulate(
            begin(records), end(records), 0.0,
            [&center](double acc, const Record::Pointer& record) {
                return std::max(Distance(center, record->data), acc);
            });
    }

    static Record* ChooseFarthest(const Records& records, Record* pivot) {
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

    static std::pair<Record*, Record*> PickPivots(const Records& records) {
        assert(records.size() >= 2);
        auto& arbitrary(records.front());
        Record* a = ChooseFarthest(records, arbitrary.get());
        Record* b = ChooseFarthest(records, a);
        return {a, b};
    }

    static std::pair<Records, Records> SplitRecord(
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

    static std::pair<Records, Records> SplitRecord(Records&& records) {
        Record *a, *b;
        std::tie(a, b) = PickPivots(records);
        return SplitRecord(std::move(records), a, b);
    }

    BallTreeBranch::Pointer BuildTreeBranch(Records&& data) {
        std::vector<float> center(CalculateCenter(data));
        double radius(CalculateRadius(data, center));
        std::pair<Records, Records> split_result(SplitRecord(std::move(data)));
        return BallTreeBranch::Create(
            std::move(center), radius, BuildTree(std::move(split_result.first)),
            BuildTree(std::move(split_result.second)));
    }

    BallTreeNode::Pointer BuildTree(Records&& records) {
        if (records.size() <= N0) {
            return BuildTreeLeaf(records);
        }
        return BuildTreeBranch(std::move(records));
    }

    /**
     * build the balltree from index file
     */
    BallTreeImpl(const Path& index_path);

    /**
     * build the balltree from plain index and vector data
     */
    BallTreeImpl(Records&& data)
        : record_storage_(storage_factory::GetMemoryOnlyStorage()) {
        // TODO
    }

    /**
     * store the balltree to an index file
     */
    bool StoreTree(const Path& index_path);

    /**
     * returns the index of the vector with the maximum inner product with the
     * vector given
     */
    int Search(const std::vector<float>& v) {
        if (not root_) {
            assert(false && "root is nullptr!");
            return -1;
        }
        detail::MIPSearcher visitor(v, record_storage_.get());
        root_->Accept(visitor);
        return visitor.ResultIndex();
    }

    /**
     * insert given vector to the balltree
     */
    bool Insert(const std::vector<float>& v);

    /**
     * delete given vector from the balltree
     */
    bool Delete(const std::vector<float>& v);

  private:
    std::unique_ptr<BallTreeNode> root_;
    std::unique_ptr<RecordStorage> record_storage_;
};

class BallTree {
  public:
    static Records ArrayToVector(
        int n, int d, float** data) {
        Records v;
        v.reserve(n);

        for (int i = 0; i < n; ++i) {
            v.push_back(Record::Create(
                i + 1, std::vector<float>(data[i], data[i] + d)));
        }
        return v;
    }
    bool buildTree(int n, int d, float** data) {
        impl_ = std::make_unique<BallTreeImpl>(ArrayToVector(n, d, data));
        return true;
    }

    bool storeTree(const char* index_path) {
        if (not impl_) {
            return false;
        }
        return impl_->StoreTree(index_path);
    }

    bool restoreTree(const char* index_path) {
        impl_ = std::make_unique<BallTreeImpl>(index_path);
        return true;
    }

    int mipSearch(int d, float* query) {
        if (not impl_) {
            return -1;
        }
        return impl_->Search(std::vector<float>(query, query + d));
    }

    // optional
    bool insertData(int d, float* data) {
        if (not impl_) {
            return -1;
        }
        return impl_->Insert(std::vector<float>(data, data + d));
    }

    // optional
    bool deleteData(int d, float* data) {
        if (not impl_) {
            return false;
        }
        return impl_->Insert(std::vector<float>(data, data + d));
    }

    // optional
    bool buildQuadTree(int n, int d, float** data) {
        return false;
    }

  private:
    std::unique_ptr<BallTreeImpl> impl_;
};

#endif
