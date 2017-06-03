#ifndef __STORAGE_H
#define __STORAGE_H

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "Utility.h"
#include "record.h"
#include "rid.h"

/**
 * low level abstraction of the 'storage' concept
 *
 * FixedLengthStorage manage the allocation/deallocation of fixed length bytes
 * in internal and external storage, ignoring what those bytes actually
 * represents
 * @tparam MaxPageInMemory set to -1 when no limitation
 */
template <
    int64_t BytesPerPage, int64_t MaxPageInMemory = -1,
    bool OmitZeroInNameOfFirstPage = true>
class FixedLengthStorage {
  public:
    FixedLengthStorage(int record_size_) {
        static_assert(
            MaxPageInMemory == -1,
            "only memory only storage can use this constructor");
    }

    FixedLengthStorage(
        int record_size, const std::string& name, const Path& dest_dir);

    Rid Put(const std::vector<Byte>& v) {
        assert(v.size() == record_size_);
    }

    Byte* Get(const Rid& rid);

    int RecordSize() const {
        return record_size_;
    }

  private:
    int record_size_;
    std::unique_ptr<std::string> name_;
    std::unique_ptr<Path> dest_dir;
    // vector<Byte*> buffer_pool_; // need better representation
};

class MemoryOnlyStorage;
class NormalStorage;

class RecordStorage {
  public:
    /**
     * stores record to the storage and returns the rid pointing to the
     * record, may first try to store in memory, then
     * flush to file if specified
     */
    virtual Rid Put(const Record&) = 0;

    /**
     * finds the record specified by rid
     * @return nullptr if not found
     */
    virtual std::unique_ptr<Record> Get(const Rid& rid) = 0;

    /**
     * dump all data to specific path,
     */
    virtual void DumpTo(const Path& dest_dir) = 0;
};

/**
 * uses only memory to store data
 */
class MemoryOnlyStorage : public RecordStorage {
  public:
    virtual Rid Put(const Record& record) override;
    virtual std::unique_ptr<Record> Get(const Rid& rid) override;
    virtual void DumpTo(const Path& dest_dir) override;
};

/**
 * uses both memory and file to store record
 */
class NormalStorage : public RecordStorage {
  public:
    NormalStorage(const Path& dest_dir);

    virtual Rid Put(const Record& record) override;
    virtual std::unique_ptr<Record> Get(const Rid& rid) override;
    virtual void DumpTo(const Path& dest_dir) override;

  private:
    Path& dest_dir_;
};

/**
 * simple storage for algorithm testing
 */
class SimpleStorage : public RecordStorage {
  public:
    virtual Rid Put(const Record& record) override {
        Rid ret(counter, 0);
        s_.insert({counter++, record});
        return ret;
    }
    virtual std::unique_ptr<Record> Get(const Rid& rid) override {
        auto iter = s_.find(rid.page_id);
        if (iter == end(s_)) {
            return nullptr;
        }
        return std::make_unique<Record>(iter->second);
    }
    virtual void DumpTo(const Path& dest_dir) override {
        // no-op
    }

  private:
    int counter = 0;
    std::unordered_map<int, Record> s_;
};

namespace storage_factory {

inline std::unique_ptr<MemoryOnlyStorage> GetMemoryOnlyStorage() {
    return nullptr;
}
inline std::unique_ptr<NormalStorage> GetNormalStorage(const Path& dest_dir) {
    return nullptr;
}

inline std::unique_ptr<SimpleStorage> GetSimpleStorage() {
    return std::make_unique<SimpleStorage>();
}

}  // namespace storage_factory
#endif


// added on June 3, 2017
// by YYJ
template <typename T>
class Slot {
  public:
    Slot() : byte_size(0) {}
    virtual bool Write(fstream out_file) = 0;
    virtual bool Read(fstream in_file) = 0;
    T Get() {
      return slot;
    }
    virtual bool Set(T& s) {
      slot = s;
    }
    int Size() {
      return byte_size;
    }
  private:
    T slot;
    int byte_size;
};

class RecordSlot : public Slot<Record> {
  public:
    RecordSlot() = default;

    RecordSlot(Record& record) {
      Set(record);
      byte_size = 4 + 4 + slot.data.size()*4;
    }
    virtual bool Write(fstream out_file) override {
      out_file.write(reinterpret_cast<char *>(&slot.index), sizeof(slot.index));
      out_file.write(reinterpret_cast<char *>(&slot.data.size()), sizeof(slot.data.size()));
      out_file.write(reinterpret_cast<char *>(&slot.data.data()), sizeof(slot.data.data()));
    }
    virtual bool Read(fstream in_file) {

    }
  private:

};
class BranchSlot : public Slot<BallTreeBranch> {
  public:
    BranchSlot() = default;
    BranchSlot(BallTreeBranch& node) {
      Set(node);
      // To do
      // change to exact bytes
      byte_size = 4 + 4 + slot.center.size()*4;
    }
    virtual bool Write(fstream out_file) override {
      out_file.write(reinterpret_cast<char *>(&slot.center.size()), sizeof(slot.center.size()));
      out_file.write(reinterpret_cast<char *>(&slot.center.data()), sizeof(slot.center.data()));
      out_file.write(reinterpret_cast<char *>(&slot.radius), sizeof(slot.radius));

      //To do
      //write two pointers into file.
      out_file.write(reinterpret_cast<char *>(&slot.radius), sizeof(slot.radius));
    }
    virtual bool Read(fstream in_file) {

    }
  private:
};
class LeafSlot : public Slot<BallTreeLeaf> {
  public:
    LeafSlot() = default;
    LeafSlot(BallTreeLeaf& node) {
      Set(node);
      byte_size = 4 + slot.center.size()*4 + 4 + 4 + slot.data.size()*8;
    }
    virtual bool Write(fstream out_file) override {
      out_file.write(reinterpret_cast<char *>(&slot.center.size()), sizeof(slot.center.size()));
      out_file.write(reinterpret_cast<char *>(&slot.center.data()), sizeof(slot.center.data()));
      out_file.write(reinterpret_cast<char *>(&slot.radius), sizeof(slot.radius));

      out_file.write(reinterpret_cast<char *>(&slot.data.size()), sizeof(slot.data.size()));
      out_file.write(reinterpret_cast<char *>(&slot.data.data()), sizeof(slot.data.data()));
    }
    virtual bool Read(fstream in_file) {

    }
  private:
};

template <typename T>
class Page {
  public:
    // max = 256
    Page() :slot_count(0), byte_size(4), max_byte(256) {}


    Slot<T>* Get(int i) {
      return slots[i];
    }

    bool canPut(Slot<T>* slot) {
      if (byte_size+slot->Size() > max_byte) return false;
      else return true;
    }
    int Put(Slot<T>* slot) {
      slots.push_back(slot);
      slot_count++;
      byte_size += slot->Size();
      return slot_count-1;
    }

    bool write(fstream out_file) {
      out_file.write(reinterpret_cast<char *>(&slot_count), sizeof(slot_count));
      for (int i = 0; i < slots.size(); i++) {
        slots[i]->Write(out_file);
      }
      return true;
    }

    int ByteSize() {
      return byte_size;
    }
    int Size() {
      return record_count;
    }
  private:
    int slot_count;
    int byte_size;
    int max_byte;
    std::vector<Slot<T>* > slots;
};

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
      file.close;
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