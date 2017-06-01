#ifndef __STORAGE_H
#define __STORAGE_H

#include <cassert>
#include <cstdint>
#include <string>
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
namespace storage_factory {

inline std::unique_ptr<MemoryOnlyStorage> GetMemoryOnlyStorage() {
    return nullptr;
}
inline std::unique_ptr<NormalStorage> GetNormalStorage(const Path& dest_dir) {
    return nullptr;
}
}
#endif
