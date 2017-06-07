#ifndef __BALL_TREE_IMPL_H
#define __BALL_TREE_IMPL_H


#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include "Utility.h"
#include "BallTreeNode.h"
#include "record.h"
#include "storage.h"
#include "MIPSearcher.h"
#include "NodeBuilder.h"

constexpr int N0 = 20;

class BallTreeImpl {
    using Records = std::vector<Record::Pointer>;

  public:
    /**
     * build the balltree from index file
     */
    BallTreeImpl(const Path& index_path);

    /**
     * build the balltree from plain index and vector data
     */
    BallTreeImpl(Records&& records);

    /**
     * functions for calculations
     */
    static std::vector<float> CalculateCenter(const Records& records);

    static double CalculateRadius(
        const Records& records, const std::vector<float>& center);

    static Record* ChooseFarthest(const Records& records, Record* pivot);

    static std::pair<Record*, Record*> PickPivots(const Records& records);

    static std::pair<Records, Records> SplitRecord(
        Records&& records, Record* a, Record* b);

    static std::pair<Records, Records> SplitRecord(Records&& records);

    /**
     *  Functions for building Ball Tree Node 
     */

    std::vector<Rid> StoreAll(const Records& records);

    BallTreeLeaf::Pointer BuildTreeLeaf(const Records& records);

    BallTreeBranch::Pointer BuildTreeBranch(Records&& data);

    BallTreeNode::Pointer BuildTree(Records&& records);


    /**
     * store the balltree to an index file
     */
    bool StoreTree(const Path& index_path);

    /**
     * returns the index of the vector with the maximum inner product with the
     * vector given
     */
    std::pair<int, double> Search(const std::vector<float>& v);

    /**
     * insert given vector to the balltree (not written now)
     */
    bool Insert(const std::vector<float>& v);

    /**
     * delete given vector from the balltree (not written now)
     */
    bool Delete(const std::vector<float>& v);


  private:
    std::unique_ptr<RecordStorage> record_storage_;
    std::unique_ptr<NodeStorage> node_storage_;
    std::unique_ptr<BallTreeNode> root_;
};

#endif