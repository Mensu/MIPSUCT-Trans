#ifndef _SLOT_H
#define _SLOT_H

#include "record.h"
#include "Utility.h"
#include "BallTreeNode.h"


// added on June 3, 2017
// by YYJ
class Slot {
  public:
    Slot() = delete;
    Slot(const Slot&) = default;
    Slot(Byte* slot, size_t byte_size, const Rid::DataType& type): slot(slot), byte_size(byte_size), type(type) {}

    bool Get(std::unique_ptr<Record>&);
    bool Get(std::unique_ptr<BallTreeBranch>&);
    bool Get(std::unique_ptr<BallTreeLeaf>&);

    bool Set(const Record&);
    bool Set(const BallTreeBranch&);
    bool Set(const BallTreeLeaf&);

    int Size() {
      return byte_size;
    }
    void SetId(unsigned int slot_id) { this->slot_id = slot_id; }

    static size_t GetSize(Rid::DataType type, int dimension);
  private:
    Byte* slot;
    int byte_size;
    unsigned slot_id;
    const Rid::DataType type;
};

#endif