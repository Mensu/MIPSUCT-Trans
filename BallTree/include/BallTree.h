#ifndef __BALL_TREE_H
#define __BALL_TREE_H

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "Utility.h"
#include "BallTreeNode.h"
#include "record.h"
#include "storage.h"
#include "BallTreeImpl.h"



class BallTree {
    using Records = std::vector<Record::Pointer>;
  public:
    static Records ArrayToVector(int n, int d, float** data);

    bool buildTree(int n, int d, float** data);

    bool storeTree(const char* index_path);

    bool restoreTree(const char* index_path);

    int mipSearch(int d, float* query);



    /**
     * Additional task (not written now)
     */

    bool insertData(int d, float* data);

    bool deleteData(int d, float* data);

    bool buildQuadTree(int n, int d, float** data);

  private:
    std::unique_ptr<BallTreeImpl> impl_;
    int dim;
};

#endif
