#define BALLTREE_TESTING_ALGORITHM

#include <gtest/gtest.h>
#include "BallTree.h"
#include "Utility.h"

using std::vector;
using std::string;
using std::pair;

namespace {

vector<Record::Pointer> ReadRecords(const string& p, int d, int n) {
    float** temp = nullptr;
    read_data(n, d, temp, p.data());
    return BallTree::ArrayToVector(n, d, temp);
}

pair<int, double> GetRightAnswer(
    const vector<Record::Pointer>& data, const Record* query) {
    double innerproduct = -1E9;
    int index;
    for (auto& rp : data) {
        double new_ip = InnerProduct(rp->data, query->data);
        if (new_ip > innerproduct) {
            innerproduct = new_ip;
            index = rp->index;
        }
    }
    return {index, innerproduct};
}

vector<pair<int, double>> GetStandardardAnswer(
    const vector<Record::Pointer>& data,
    const vector<Record::Pointer>& queries) {
    vector<pair<int, double>> answers(queries.size());
    std::transform(
        begin(queries), end(queries), begin(answers),
        [&data](const Record::Pointer& q) {
            return GetRightAnswer(data, q.get());
        });
    return answers;
}

}  // anonymous namespace

constexpr int kRecordSize = 500;
constexpr int kQuerySize = 100;
constexpr int kDimension = 50;
constexpr char kDataset[] = "Netflix";

string DataPath(const std::string& dataset = kDataset) {
    return string("./") + dataset + "/src/dataset.txt";
}

string QueryPath(const std::string& dataset = kDataset) {
    return string("./") + dataset + "/src/dataset.txt";
}

class TreeAlgorithmTest : public testing::Test {
  public:
    virtual void SetUp() {
        records_ = ReadRecords(DataPath(), kDimension, kRecordSize);
        queries_ = ReadRecords(QueryPath(), kDimension, kQuerySize);
        standard_answers_ = GetStandardardAnswer(records_, queries_);
    }

  protected:
    vector<Record::Pointer> records_;
    vector<Record::Pointer> queries_;
    vector<pair<int, double>> standard_answers_;
};

TEST(MathPrimitiveTest, TestInnerProduct) {
    vector<int> v1{1, 2, 3, 4}, v2{2, 3, 4, 5};
    double innerproduct = InnerProduct(v1, v2);
    ASSERT_DOUBLE_EQ(innerproduct, 40.0);
    double innerproduct2 = InnerProduct(vector<int>(), vector<int>());
    ASSERT_DOUBLE_EQ(innerproduct2, 0.0);
}

TEST(MathPrimitiveTest, TestDistance) {
    vector<int> v1{0, 0}, v2{3, 4}, v3{3, 4};
    double dist = Distance(v1, v2);
    ASSERT_DOUBLE_EQ(dist, 5.0);
    double dist2 = Distance(vector<int>(), vector<int>());
    ASSERT_DOUBLE_EQ(dist2, 0.0);
    double dist3 = Distance(v2, v3);
    ASSERT_DOUBLE_EQ(dist3, 0.0);

    vector<int> v4{1, 2, 3, 4}, v5{5, 6, 7, 8};
    double dist4 = Distance(v4, v5);
    ASSERT_DOUBLE_EQ(dist4, 8.0);
}

std::ostream& operator<<(std::ostream& os, pair<int, double>& p) {
    os << '(' << p.first << ',' << p.second << ')';
    return os;
}

TEST_F(TreeAlgorithmTest, HelloWorld) {
    ASSERT_EQ(records_.size(), kRecordSize);
    ASSERT_EQ(queries_.size(), kQuerySize);
    ASSERT_EQ(standard_answers_.size(), kQuerySize);
    for (auto standard_answer : standard_answers_) {
        std::cout << standard_answer << ' ';
    }
    std::cout << '\n';
}

vector<float> CalculateTestCenter(const vector<Record::Pointer>& records) {
    vector<float> center_answer(records.front()->Size(), 0);
    for (auto& record : records) {
        for (int i = 0; i < record->Size(); ++i) {
            center_answer[i] += record->data[i];
        }
    }
    for (auto& x : center_answer) {
        x /= records.size();
    }
    return center_answer;
}

TEST_F(TreeAlgorithmTest, TestCenter) {
    vector<float> center1(BallTreeImpl::CalculateCenter(records_));
    std::reverse(begin(records_), end(records_));
    vector<float> center2(BallTreeImpl::CalculateCenter(records_));
    ASSERT_EQ(center1.size(), kDimension);
    ASSERT_EQ(center2.size(), kDimension);
    for (int i = 0; i < center1.size(); ++i) {
        EXPECT_NEAR(center1[i], center2[i], 0.00001);
    }
    vector<float> center_answer(CalculateTestCenter(records_));
    for (int i = 0; i < center_answer.size(); ++i) {
        EXPECT_NEAR(center1[i], center_answer[i], 0.00001);
    }
}

TEST_F(TreeAlgorithmTest, TestRadius) {
    std::vector<float> center(CalculateTestCenter(records_));
    double radius = BallTreeImpl::CalculateRadius(records_, center);
    bool radius_found = false;
    for (auto& record : records_) {
        double dist = Distance(record->data, center);
        EXPECT_TRUE(dist <= radius);
        if (std::fabs(radius - dist) <= 0.00001) {
            radius_found = true;
        }
    }
    EXPECT_TRUE(radius_found);
}

TEST_F(TreeAlgorithmTest, TestSearch) {
    BallTreeImpl ball_tree(std::move(records_));
    for (int i = 0; i < queries_.size(); ++i) {
        pair<int, double> search_answer = ball_tree.Search(queries_[i]->data);
        std::cout << search_answer << ' ';
        EXPECT_DOUBLE_EQ(search_answer.second, standard_answers_[i].second);
    }
    std::cout << '\n';
}
