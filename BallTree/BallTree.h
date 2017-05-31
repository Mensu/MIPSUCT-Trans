#ifndef __BALL_TREE_H
#define __BALL_TREE_H

#define N0 20

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

struct Record {
    Record(int index, std::vector<float>&& data)
        : index(index), data(std::move(data)) {}

    Record(Record&&) = default;
    Record(const Record&) = default;
    Record& operator=(Record&&) = default;
    Record& operator=(const Record&) = default;
    ~Record() = default;

    int index;
    std::vector<float> data;
};

class BallTreeImpl {
  public:
    /**
     * build the balltree from index file
     */
    BallTreeImpl(const std::string& index_path);

    /**
     * build the balltree from plain index and vector data
     */
    BallTreeImpl(const std::vector<Record>& data) {
        // TODO 
        for (const auto& r : data) {
            std::cout<< r.index <<  ' ' << r.data.size() << '\n';
            for (const auto& d : r.data) {
                std::cout << d << ' ';
            }
            std::cout<< '\n';
        }
    }

    /**
     * store the balltree to an index file
     */
    bool StoreTree(const std::string& index_path);

    /**
     * returns the index of the vector with the maximum inner product with the
     * vector given
     */
    bool Search(const std::vector<float>& v);

    /**
     * insert given vector to the balltree
     */
    bool Insert(const std::vector<float>& v);

    /**
     * delete given vector from the balltree
     */
    bool Delete(const std::vector<float>& v);

};

class BallTree {
  public:
    bool buildTree(int n, int d, float** data) {
        std::vector<Record> v;
        v.reserve(n);

        for (int i = 0; i < n; ++i) {
            v.push_back(Record(i + 1, std::vector<float>(data[i], data[i] + d)));
        }

        impl_ = std::make_unique<BallTreeImpl>(v);
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
