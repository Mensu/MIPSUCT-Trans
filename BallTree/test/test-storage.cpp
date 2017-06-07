#include "rid.h"
#include "record.h"
#include "storage.h"
#include <iostream>

int main() {
  constexpr int dimension = 100;
  std::vector<float> v;
  v.reserve(dimension);
  for (int index = 0; index < dimension; ++index) {
    v.push_back(index);
  }
  Record record(0, std::move(v));
  FixedLengthStorage<1024, Rid::record, 5> storage(sizeof(size_t) + sizeof(int) + record.Size() * sizeof(float), "name.txt", "./");
  std::vector<Rid> rids;
  for (int index = 0; index < 20; ++index) {
    auto rid = storage.Put(record);
    auto ptr = storage.Get<Record>(rid);
    rids.push_back(rid);
    continue;
  }
  std::cout << "hhhh" << std::endl;

  for (int index = 9; index > -1; --index) {
    auto ptr = storage.Get<Record>(rids[index]);
    rids.pop_back();
    continue;
  }
  return 0;
}