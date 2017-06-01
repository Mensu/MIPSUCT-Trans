#include <gtest/gtest.h>
#include "BallTree.h"
#include "Utility.h"

using std::vector;
using std::string;

namespace {

vector<Record::Pointer> ReadRecords(const string& p, int d, int n) {
    float** temp = nullptr;
    read_data(n, d, temp, p.data());
    return BallTree::ArrayToVector(n, d, temp);
}

int GetRightAnswer(const vector<Record::Pointer>& data, const Record* query) {
    double innerproduct = -1E9;
    int index;
    for (auto& rp : data) {
        double new_ip = InnerProduct(rp->data, query->data);
        if (new_ip > innerproduct) {
            innerproduct = new_ip;
            index = rp->index;
        }
    }
    return index;
}

vector<int> GetStandardardAnswer(
    const vector<Record::Pointer>& data,
    const vector<Record::Pointer>& queries) {
    vector<int> answers(queries.size());
    std::transform(
        begin(queries), end(queries), begin(answers),
        [&data](const Record::Pointer& q) { return GetRightAnswer(data, q.get()); });
    return answers;
}

}  // anonymous namespace

class TreeAlgorithmTest : public testing::Test {
  public:
    virtual void SetUp() {
        records_ = ReadRecords("./Netflix/src/dataset.txt", 50, 200);
        queries_ = ReadRecords("./Netflix/src/query.txt", 50, 60);
        standard_answers_ = GetStandardardAnswer(records_, queries_);
    }

  protected:
    vector<Record::Pointer> records_;
    vector<Record::Pointer> queries_;
    vector<int> standard_answers_;
};

TEST_F(TreeAlgorithmTest, HelloWorld) {
    ASSERT_TRUE(records_.size() == 200);
    ASSERT_TRUE(queries_.size() == 60);
    ASSERT_TRUE(standard_answers_.size() == 60);
}
