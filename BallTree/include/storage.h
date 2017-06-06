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

struct BallTreeNode;

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
    using Bitset = std::vector<bool>;
    using Pages = std::unordered_map<int, Page>;
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
  
    bool pageInMemory(int page_id) {
        return m_frames.find(page_id) != m_frames.end();
    }
    
    /**
     * @Description Swaping the page in frame when the frames
     * @Return the swap-out page_id
     */
    int swapPageOut() {

    }

    bool framesFull() {

    }


  private:
    int record_size_;
    std::unique_ptr<std::string> name_;
    std::unique_ptr<Path> dest_dir;
    Bitset m_dirty_map;
    Pages m_frames;
};


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

    virtual ~RecordStorage() {}
};
/**
 * storage store node
 */
class NodeStorage {
  public:
    BallTreeNode* Get(Rid rid) {
      return nullptr;
    }
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
