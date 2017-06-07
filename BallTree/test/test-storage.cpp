#include "rid.h"
#include "record.h"
#include "storage.h"
#include <iostream>

using FloatVector = std::vector<float>;
constexpr int dimension = 100;

FloatVector getVectorStartingWith(int start) {
  std::vector<float> v;
  v.reserve(dimension);
  for (int index = 0; index < dimension; ++index) {
    v.push_back(start + index);
  }
  return std::move(v);
}

int main() {
  Record record(0, std::move(getVectorStartingWith(0)));
  auto slotSize = sizeof(size_t) + sizeof(int) + record.Size() * sizeof(float);
  FixedLengthStorage<1024, Rid::record, 5> storage(slotSize, "name.txt", "./tt/");
  std::vector<Rid> rids;
  for (int index = 0; index < 20; ++index) {
    Record record(0, std::move(getVectorStartingWith(index * dimension)));
    auto rid = storage.Put(record);
    rids.push_back(rid);
    continue;
  }

  for (int index = 15; index >= 0; --index) {
    auto ptr = storage.Get<Record>(rids[index]);
    std::cout << ptr->data[0] << " " << ptr->data[1] << std::endl;
    continue;
  }
  return 0;
}
