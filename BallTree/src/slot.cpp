#include "slot.h"
#include <algorithm>
/**
 * A slot of Record
 * +-------+-----------+-------------------+
 * |  int  |   size_t  | float [data_size] |
 * +-------+-----------+-------------------+
 * | index | data_size |    vector data    |
 * +-------+-----------+-------------------+
 */
bool Slot::Get(std::unique_ptr<Record>& pointer) {
  if (type != Rid::record) return false;
  int* index = reinterpret_cast<int*>(slot);
  const size_t& size = *reinterpret_cast<size_t*>(slot + sizeof(int));
  float* data_start =
      reinterpret_cast<float*>(slot + sizeof(int) + sizeof(size_t));
  std::vector<float> data_vec(data_start, data_start + size);
  pointer = Record::Create(*index, std::move(data_vec));
  return true;
}

/**
 * A slot of BallTreeBranch
 * +-------------+---------------------+--------+------+-------+
 * |    size_t   | float [center_size] | double |  Rid |  Rid  |
 * +-------------+---------------------+--------+------+-------+
 * | center_size |    vector center    | radius | left | right |
 * +-------------+---------------------+--------+------+-------+
 */
bool Slot::Get(std::unique_ptr<BallTreeBranch>& pointer) {
  if (type != Rid::branch) return false;
  const size_t& center_size = *reinterpret_cast<size_t*>(slot);
  float* center_begin = reinterpret_cast<float*>(slot + sizeof(double));
  Byte* radius_begin = reinterpret_cast<Byte*>(center_begin + center_size);

  const double& radius = *reinterpret_cast<double*>(radius_begin);
  auto left = *reinterpret_cast<Rid*>(radius_begin + sizeof(double));
  auto right = *reinterpret_cast<Rid*>(radius_begin + sizeof(double) + sizeof(Rid));
  std::vector<float> center(center_begin, center_begin + center_size);
  pointer = BallTreeBranch::Create(std::move(center), radius, nullptr, nullptr, *left, *right);
  return true;
}

/**
 * A slot of BallTreeLeaf
 * +-------------+---------------------+--------+----------+----------------+
 * |    size_t   | float [center_size] | double |  size_t  | Rid [rid_size] |
 * +-------------+---------------------+--------+----------+----------------+
 * | center_size |    vector center    | radius | rid_size |  vector rids   |
 * +-------------+---------------------+--------+----------+----------------+
 */
bool Slot::Get(std::unique_ptr<BallTreeLeaf>& pointer) {
  if (type != Rid::leaf) return false;
  const size_t& center_size = *reinterpret_cast<size_t*>(slot);
  float* center_begin = reinterpret_cast<float*>(slot + sizeof(size_t));
  Byte* radius_begin = reinterpret_cast<Byte*>(center_begin + center_size);
  const size_t& rid_size =
      *reinterpret_cast<size_t*>(radius_begin + sizeof(double));
  Rid* rid_begin =
      reinterpret_cast<Rid*>(radius_begin + sizeof(double) + sizeof(size_t));

  const double& radius = *reinterpret_cast<double*>(radius_begin);
  std::vector<float> center(center_begin, center_begin + center_size);
  std::vector<Rid> rids(rid_begin, rid_begin + rid_size);
  pointer = BallTreeLeaf::Create(std::move(center), radius, std::move(rids));
  return true;
}
/**
 * A slot of Record
 * +-------+-----------+-------------------+
 * |  int  |   size_t  | float [data_size] |
 * +-------+-----------+-------------------+
 * | index | data_size |    vector data    |
 * +-------+-----------+-------------------+
 */
bool Slot::Set(const Record& record) {
  if (type != Rid::record) return false;
  assert(record.Size() * sizeof(float) + sizeof(size_t) + sizeof(int) ==
         byte_size);
  int* index_addr = reinterpret_cast<int*>(slot);
  size_t* size_addr = reinterpret_cast<size_t*>(slot + sizeof(int));
  float* data_begin =
      reinterpret_cast<float*>(slot + sizeof(int) + sizeof(size_t));

  *index_addr = record.index;
  *size_addr = record.Size();
  std::transform(record.data.begin(), record.data.end(), data_begin,
                 [](const auto& data) { return data; });
  return true;
}

/**
 * A slot of BallTreeBranch
 * +-------------+---------------------+--------+------+-------+
 * |    size_t   | float [center_size] | double |  Rid |  Rid  |
 * +-------------+---------------------+--------+------+-------+
 * | center_size |    vector center    | radius | left | right |
 * +-------------+---------------------+--------+------+-------+
 */
bool Slot::Set(const BallTreeBranch& branch) {
  if (type != Rid::branch) return false;
  assert(sizeof(size_t) + sizeof(float) * branch.center.size() +
             sizeof(Rid) * 2 + sizeof(double) <=
         byte_size);
  size_t* center_size = reinterpret_cast<size_t*>(slot);
  float* center_begin = reinterpret_cast<float*>(slot + sizeof(size_t));
  std::transform(branch.center.begin(), branch.center.end(), center_begin,
                 [](const auto& data) { return data; });
  double* radius = reinterpret_cast<double*>(center_begin + *center_size);
  *center_size = branch.center.size();
  Rid* left_addr =
      reinterpret_cast<Rid*>(center_begin + *center_size + sizeof(double));
  Rid* right_addr = left_addr + 1;

  *radius = branch.radius;
  *left_addr = branch.r_left;
  *right_addr = branch.r_right;

  return true;
}
/**
 * A slot of BallTreeLeaf
 * +-------------+---------------------+--------+----------+----------------+
 * |    size_t   | float [center_size] | double |  size_t  | Rid [rid_size] |
 * +-------------+---------------------+--------+----------+----------------+
 * | center_size |    vector center    | radius | rid_size |  vector rids   |
 * +-------------+---------------------+--------+----------+----------------+
 */
bool Slot::Set(const BallTreeLeaf& leaf) {
  if (type != Rid::leaf) return false;
  assert(sizeof(size_t) * 2 + sizeof(double) +
             sizeof(float) * leaf.center.size() +
             sizeof(Rid) * leaf.data.size() <=
         byte_size);
  size_t* center_size = reinterpret_cast<size_t*>(slot);
  *center_size = leaf.center.size();
  float* center_begin = reinterpret_cast<float*>(slot + sizeof(size_t));
  Byte* radius = reinterpret_cast<Byte*>(center_begin + *center_size);
  size_t* rid_size = reinterpret_cast<size_t*>(radius + sizeof(double));
  Rid* rid_begin =
      reinterpret_cast<Rid*>(radius + sizeof(double) + sizeof(size_t));

  std::transform(leaf.center.begin(), leaf.center.end(), center_begin,
                 [](const auto& data) { return data });
  std::transform(leaf.data.begin(), leaf.data.end(), rid_begin,
                 [](const auto& data) { return data; });
  *reinterpret_cast<double*>(radius) = leaf.radius;
  *rid_size = leaf.data.size();
  return true;
}