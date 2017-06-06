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
    bool Get(std::unique_ptr<BallTreeBranch>&, Rid& left, Rid& right);
    bool Get(std::unique_ptr<BallTreeLeaf>&);

    bool Set(const Record&);
    bool Set(const BallTreeBranch&, const Rid& left, const Rid& right);
    bool Set(const BallTreeLeaf&);

    int Size() {
      return byte_size;
    }
    void SetId(unsigned int slot_id) { this->slot_id = slot_id; }
  private:
    Byte* slot;
    int byte_size;
    unsigned slot_id;
    const Rid::DataType type;
};

#if 0
template <typename T>
class BasicStorage {
  public:
    BasicStorage(std::string n, Path& dir)
        : page_count(0), dest_dir(dir), name(n) {
        pages.push_back(Page(0));
    }

    virtual Rid Put(Slot<T>* slot) {
        Page* current_page = pages.back();
        if (!current_page.canPut(slot)) {
          pages.push_back(Page(++page_count));
          current_page = pages.back();
        }

        int slot_num = current_page.Put(slot);
        return Rid(page_count, slot_num);
    }


    Slot<T>* Get(Rid& rid) {
      if (rid.page_id > page_count) return nullptr;
      if (rid.slot_id > pages[rid.page_id].Size()) return nullptr;

      return pages[rid.page_id].Get(rid.slot_id);
    }

    bool saveAll() {
      return saveIndex() && saveData();
    }

    bool saveIndex() {
      fstream file(dest_dir+"index", ios::out | ios::binary);
      file.write(reinterpret_cast<char *>(&page_size), sizeof(page_size));
      file.close();
      return true;
    }

    bool saveData() {
      for (int i = 0; i < pages.size(); ++i)  {
        stringstream count(i);
        std::string fileName = name + count.str();
        fstream out_file(dest_dir + fileName, ios::out | ios::binary);
        pages[i]->Write(out_file);
        out_file.close();
      }
      return true;
    }

    bool loadAll() {
      loadIndex();
      loadData();
    }

    bool loadIndex() {
      fstream file(dest_dir+"index", ios::in | ios::binary);
      file.read(reinterpret_cast<char *>(&page_size), sizeof(page_size));
      file.close;
    }

    bool loadData() {
      // To do
    }

  protected:
    BasicStorage() = default;
    std::string name;
    Path dest_dir;
  private:
    int page_count;
    std::vector<Page> pages;
};

class BallTreeLeafStorage : public BasicStorage<BallTreeLeaf> {
  public:
    BallTreeLeafStorage(Path& dir) :dest_dir(dir), name("BallTreeLeaf") {}
    Rid Put(LeafSlot& slot) {
      BasicStorage<BallTreeLeaf>* that = *this;
      that->Put(reinterpret_cast<Slot<BallTreeLeaf>*>(&slot));
    }
    bool Save() {
      return saveAll();
    }
  private:
};


class BallTreeBranchStorage : public BasicStorage {
  public:
    BallTreeBranchStorage(Path& dir) :dest_dir(dir), name("BallTreeBranch") {}
    Rid Put(BranchSlot& slot) {
      BasicStorage<BallTreeBranch>* that = *this;
      return that->Put(reinterpret_cast<Slot<BallTreeBranch>*>(&slot));
    }
    bool Save() {
      return saveAll();
    }
  private:
};

class RecordStorage : public BasicStorage {
  public:
    RecordStorage(Path& dir) :dest_dir(dir), name("Record") {}
    Rid Put(RecordSlot& slot) {
      BasicStorage<RecordSlot>* that = *this;
      return that->Put(reinterpret_cast<Slot<RecordSlot>*>(&slot));
    }
    bool Save() {
      return saveAll();
    }
  private:
};

class Storage {
  public:
    Storage(Path& dir) :leaf_storage(dir), branch_storage(dir), record_storage(dir) {}
    Rid& PutBranch(BallTreeBranch& slot) {
      return branch_storage.Put(slot);
    }
    Slot<BallTreeBranch>* GetBranch(Rid& rid) {
      return branch_storage.Get(rid);
    }

    Rid& PutLeaf(BallTreeLeaf& slot) {
      return branch_storage.Put(slot);
    }
    Slot<BallTreeLeaf>* GetLeaf(Rid& rid) {
      return leaf_storage.Get(rid);
    }

    Rid& PutRecord(Record& slot) {
      return branch_storage.Put(slot);
    }
    Slot<Record>* GetRecord(Rid& rid) {
      return branch_storage.Get(rid);
    }
  private:
    BallTreeLeafStorage leaf_storage;
    BallTreeBranchStorage branch_storage;
    RecordStorage record_storage;
};

#endif

#endif