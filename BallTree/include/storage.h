#ifndef __STORAGE_H
#define __STORAGE_H

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include "Utility.h"
#include "record.h"
#include "rid.h"
#include "page.h"

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
    int64_t BytesPerPage,
    Rid::DataType DataType,
    int64_t MaxPageInMemory = -1,
    bool OmitZeroInNameOfFirstPage = true>
class FixedLengthStorage {
    using BitSet = std::vector<bool>;
    using PagePtr = std::shared_ptr<Page>;
  public:
    FixedLengthStorage(int record_size) {
        static_assert(
            MaxPageInMemory == -1,
            "only memory only storage can use this constructor");
    }

    FixedLengthStorage(
        int record_size, const std::string& name, const Path& dest_dir)
        : record_size(record_size),
          name(name),
          dest_dir(dest_dir),
          page_num(0),
          is_referenced(MaxPageInMemory, false),
          is_dirty(MaxPageInMemory, false),
          frames(MaxPageInMemory, nullptr),
          buffer_ptr(new Byte[buffer_size]()) {
        static_assert(MaxPageInMemory > 0, "MaxPageInMemory should be > 0 when using this constructor");
        auto fs = getFs();
        if (fs.tellg() == 0) {
            fs.write(reinterpret_cast<char *>(&this->page_num), sizeof(this->page_num));
        } else {
            fs.read(reinterpret_cast<char *>(&this->page_num), sizeof(this->page_num));
        }
    }

    template <typename T>
    Rid Put(const T &data) {
        auto cur_size = this->page_to_frame_map.size();
        PagePtr non_full_page_ptr = nullptr;
        for (auto &pair : this->page_to_frame_map) {
            auto &cur_page_ptr = this->frames[pair.second];
            if (cur_page_ptr->isFull()) continue;
            non_full_page_ptr = cur_page_ptr;
            break;
        }
        // all pages are full
        if (not non_full_page_ptr) {
            // create new page
            auto frame_id = this->swapVictimPageOut();
            non_full_page_ptr = std::make_shared<Page>(this->page_num, this->record_size, DataType, this->getFrameAddr(frame_id), this->page_size_in_k);
            this->initNewPage(non_full_page_ptr, frame_id);
            this->is_dirty[frame_id] = true;
            ++this->page_num;
        }
        auto page_id = non_full_page_ptr->PageId();
        auto frame_id = this->page_to_frame_map.at(page_id);
        this->is_referenced[frame_id] = true;
        auto insert_result = non_full_page_ptr->insert();
        auto cur_slot = std::get<1>(insert_result);
        cur_slot.Set(data);
        return std::get<0>(insert_result);
    }

    template <typename T>
    std::unique_ptr<T> Get(const Rid &rid) {
        if (not this->pageInMemory(rid.page_id)) {
            this->swapPageIn(rid.page_id, this->swapVictimPageOut());
        }
        auto frame_id = this->page_to_frame_map[rid.page_id];
        auto &cur_page_ptr = this->frames[frame_id];
        this->is_referenced[frame_id] = true;
        auto cur_slot = cur_page_ptr->select(rid.slot_id);
        std::unique_ptr<T> ptr;
        cur_slot.Get(ptr);
        return std::move(ptr);
    }

    ~FixedLengthStorage() {
        auto fs = getFs();
        for (auto &pair : this->page_to_frame_map) {
            if (not this->is_dirty[pair.second]) continue;
            auto &cur_page_ptr = this->frames[pair.second];
            cur_page_ptr->sync(fs);
        }
    }

    int RecordSize() const {
        return this->record_size;
    }
  private:

    bool pageInMemory(int page_id) {
        return this->page_to_frame_map.find(page_id) != this->page_to_frame_map.end();
    }

    /**
     * @Description Swaping the page in frame when the frames
     * @Return the swap-out frame_id
     */
    std::size_t swapVictimPageOut() {
        auto victim_page_id = this->findVictimPage();
        auto fs = getFs();
        // swap out
        std::size_t frame_id = 0;
        if (victim_page_id == -1) {
            frame_id = this->page_to_frame_map.size();
        } else {
            frame_id = this->page_to_frame_map.at(victim_page_id);
            if (this->is_dirty[frame_id]) {
                this->swapPageOut(frame_id, fs);
            }
            this->page_to_frame_map.erase(victim_page_id);
        }
        return frame_id;
    }

    void swapPageOut(std::size_t frame_id, std::ostream &out) {
        auto page_ptr = this->frames[frame_id];
        auto page_position = this->begin_pos + page_ptr->PageId() * this->record_size;
        out.seekp(page_position);
        page_ptr->sync(out);
    }

    void swapPageIn(int page_in_id, std::size_t frame_id) {
        auto fs = getFs();
        return this->swapPageIn(page_in_id, frame_id, fs);
    }

    void swapPageIn(int page_in_id, std::size_t frame_id, std::istream &in) {
        auto page_position = this->begin_pos + page_in_id * this->record_size;
        in.seekg(page_position);
        auto new_page_ptr = std::make_shared<Page>(page_in_id, in, this->getFrameAddr(frame_id), this->page_size_in_k);
        this->initNewPage(new_page_ptr, frame_id);
        this->is_dirty[frame_id] = false;
    }

    void initNewPage(PagePtr new_page_ptr, std::size_t frame_id) {
        this->frames[frame_id] = new_page_ptr;
        this->is_referenced[frame_id] = true;
        this->page_to_frame_map.insert({ new_page_ptr->PageId(), frame_id });
    }

    int findVictimPage() {
        static_assert(MaxPageInMemory > 0, "MaxPageInMemory should be > 0 when calling findVictimPage");
        if (not this->framesFull()) {
            return -1;
        }

        static int64_t victim_frame_id = 0;
        for (std::size_t index = 0; index < MaxPageInMemory; ++index) {
            if (this->is_referenced[victim_frame_id]) {
                this->is_referenced[victim_frame_id] = false;
            } else {
                return this->frames[victim_frame_id]->PageId();
            }

            victim_frame_id = (victim_frame_id + 1) % MaxPageInMemory;
        }
        return this->frames[victim_frame_id]->PageId();
    }

    bool framesFull() const {
        return this->page_to_frame_map.size() == MaxPageInMemory;
    }

    std::fstream getFs() const {
        std::fstream fs(this->dest_dir + this->name, openmode);
        return std::move(fs);
    }

    inline Byte *getFrameAddr(std::size_t frame_id) const {
        return reinterpret_cast<Byte *>(reinterpret_cast<Byte (*)[page_size]>(this->buffer_ptr.get()) + frame_id);
    }

  private:
    int record_size;
    std::string name;
    Path dest_dir;
    int page_num;

    BitSet is_referenced;
    BitSet is_dirty;
    std::vector<PagePtr> frames;
    std::unordered_map<int, std::size_t> page_to_frame_map;
    std::unique_ptr<Byte> buffer_ptr;

    static constexpr std::ios_base::openmode openmode = std::ios::binary | std::ios::out | std::ios::app;
    static constexpr std::size_t page_size_in_k = (BytesPerPage + 1023) / 1024;
    static constexpr std::size_t page_size = page_size_in_k * 1024;
    static constexpr std::size_t buffer_size = page_size * MaxPageInMemory;
    static constexpr std::size_t begin_pos = sizeof(page_num) + sizeof(Rid);
    // page_num and Rid of Tree Root
};

// 不知道为什么要加下面这一坨才能沃克
template <int64_t a, Rid::DataType b, int64_t c, bool d>
constexpr std::size_t FixedLengthStorage<a, b, c, d>::page_size_in_k;


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

inline std::unique_ptr<SimpleStorage> GetMemoryOnlyStorage() {
    return nullptr;
}
inline std::unique_ptr<SimpleStorage> GetNormalStorage(const Path& dest_dir) {
    return nullptr;
}

inline std::unique_ptr<SimpleStorage> GetSimpleStorage() {
    return std::make_unique<SimpleStorage>();
}

}  // namespace storage_factory
#endif
