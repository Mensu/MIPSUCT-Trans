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
 */
template <int64_t BytesPerPage, bool OmitZeroInNameOfFirstPage = true>
class FixedLengthStorage {
  public:
    FixedLengthStorage(const std::string& name, int record_size);

    Rid Store(const std::vector<Byte>& v) {
        assert(v.size() == record_size_);
    }

    Byte* Retrieve(const Rid& rid);

    int RecordSize() const {
        return record_size_;
    }

  private:
    int record_size_;
    std::string name_;
    // vector<char*> buffer_pool_; // need better representation
};

class Storage {
  public:
    /**
     * initialize a storage without the external storage path specified,
     * all store action happens in main memory until explicitly set the storage
     * directory
     */
    Storage();

    /**
     * initialize the storage with the external directory path, enabling the
     * external storage
     */
    Storage(const Path& storage_dir);

    /**
     * stores record to the storage and returns the rid pointing to the
     * record, may first try to store in memory, then
     * flush to file if specified
     */
    Rid SaveRecord(const Record&) {
        // TODO
        return Rid(0, 0);
    }

    /**
     * finds the record specified by rid
     * @return nullptr if not found
     */
    std::unique_ptr<Record> GetRecord(const Rid& rid) {
        return nullptr;
    }

  private:
    std::unique_ptr<Path> storage_dir_;
};

#endif
