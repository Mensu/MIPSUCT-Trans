#include "BallTree.h"
#include "Utility.h"
#include <chrono>
#include <iostream>
#define NETFLIX

#ifdef MNIST
char dataset[L] = "Mnist";
int n = 600, d = 50;
int qn = 1000;
#endif

#ifdef YAHOO
char dataset[L] = "Yahoo";
int n = 624, d = 300;
int qn = 1000;
#endif

#ifdef NETFLIX
char dataset[L] = "Netflix";
int n = 17770;
int d = 50;
int qn = 1000;
#endif

constexpr int kQN = 1000;

template <const char *Name, int N, int D>
struct DataSet {
    static constexpr const char *kName = Name;
    static constexpr int kDataScale = N;
    static constexpr int kDimension = D;
};
constexpr char kNetflix[] = "Netflix";
constexpr char kYahoo[] = "Yahoo";
constexpr char kMnist[] = "Mnist";

using Netflix = DataSet<kNetflix, 17770, 50>;
using Yahoo =
    DataSet<kYahoo, 10000, 300>; // shrinked data scale, 624000 -> 62400
using Mnist = DataSet<kMnist, 60000, 50>;
using Records = std::vector<Record::Pointer>;
namespace {

using namespace std::string_literals;

std::string QueryPath(const char *dataset) {
    return dataset + "/src/query.txt"s;
}
std::string DataPath(const char *dataset) {
    return dataset + "/src/dataset.txt"s;
}
std::string IndexPath(const char *dataset) { return dataset + "/index/"s; }

template <
    typename Duration = std::chrono::milliseconds, typename Func, typename... Args>
Duration Time(const Func &f, Args &&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    f(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<Duration>(end - start);
}

template <typename F>
void TimeAndPrint(const F &f, const std::string &prologue = "") {
    std::cout << prologue;
    auto time = Time(f);
    std::printf("DONE.\n It took %lf seconds\n\n", time.count() / 1000.);
}

template <
    template <const char *, int, int> class DataSet, const char *Name,
    int Scale, int Dimension>
float **TestBuildTree(DataSet<Name, Scale, Dimension>, BallTree &tree) {
    std::string data_path(DataPath(Name));
    float **data(nullptr);
    read_data(Scale, Dimension, data, data_path.data());
    TimeAndPrint(
        [&] { tree.buildTree(Scale, Dimension, data); },
        "Building BallTree... ");
    return data;
}

template <
    template <const char *, int, int> class DataSet, const char *Name,
    int Scale, int Dimension>
void TestStoreTree(DataSet<Name, Scale, Dimension>, BallTree &tree) {
    std::string index_path(IndexPath(Name));
    TimeAndPrint(
        [&] { tree.storeTree(index_path.data()); },
        "Storing BallTree to " + index_path + " ... ");
}

template <
    template <const char *, int, int> class DataSet, const char *Name,
    int Scale, int Dimension>
void TestRestoreTree(DataSet<Name, Scale, Dimension>, BallTree &tree) {
    std::string index_path(IndexPath(Name));
    TimeAndPrint(
        [&] { tree.restoreTree(index_path.data()); },
        "Restoring BallTree from " + index_path + " ... ");
}

void CheckResult(const Records &data, const Record &query, int answer) {
    assert(data[answer - 1]->index == answer);
    double res_innerproduct = InnerProduct(data[answer - 1]->data, query.data);
    for (auto &datap : data) {
        double test_innerproduct = InnerProduct(datap->data, query.data);
        if (test_innerproduct > res_innerproduct and datap->index != answer) {
            std::printf(
                "WARNING! Query %d: Record %d has greater inner product(%lf), "
                "than answer(record: %d)(%lf)\n",
                query.index, datap->index, test_innerproduct, answer, res_innerproduct);
            return;
        }
    }
}

void CheckResults(
    const std::vector<int> &result, const Records &data,
    const Records &queries) {
    for (int i = 0; i < kQN; ++i) {
        CheckResult(data, *queries[i], result[i]);
    }
}

template <
    template <const char *, int, int> class DataSet, const char *Name,
    int Scale, int Dimension>
void TestSearchTree(
    DataSet<Name, Scale, Dimension>, BallTree &tree, float **data) {
    std::string query_path(QueryPath(Name));
    float **queries(nullptr);
    read_data(Scale, Dimension, queries, query_path.data());
    std::vector<int> result;
    result.reserve(kQN);
    TimeAndPrint(
        [&] {
            for (int i = 0; i < kQN; ++i) {
                result.push_back(tree.mipSearch(Dimension, queries[i]));
            }
        },
        "Searching " + std::to_string(kQN) + " " + std::to_string(Dimension) +
            "-dimension vector in " + std::to_string(Scale) + "records ... ");
    Records data_records(BallTree::ArrayToVector(Scale, Dimension, data)),
        query_records(BallTree::ArrayToVector(kQN, Dimension, queries));
    std::printf("Checking Results...\n");
    CheckResults(result, data_records, query_records);
    std::printf("Done.\n");
}

template <
    template <const char *, int, int> class DataSet, const char *Name,
    int Scale, int Dimension>
void TestDataSet(DataSet<Name, Scale, Dimension> tag) {
    std::printf("Testing %s dataset, with Scale = %d, Dimension = %d\n\n",Name, Scale, Dimension );
    BallTree tree;
    float** data = TestBuildTree(tag, tree);
    TestStoreTree(tag, tree);
    BallTree tree2;
    TestRestoreTree(tag, tree2);
    TestSearchTree(tag, tree2, data);
    std::printf("\n");
}

template <typename... DataSets>
void TestDataSets() {
    auto a = {(TestDataSet(DataSets()), 0)...};
}


} // anonymous namespace

int main() {
    TestDataSets<Yahoo, Netflix, Mnist>();
}
