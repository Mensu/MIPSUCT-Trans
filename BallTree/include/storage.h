#ifndef __STORAGE_H
#define __STORAGE_H

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>
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
    FixedLengthStorage(int slot_size) {
        static_assert(
            MaxPageInMemory == -1,
            "only memory only storage can use this constructor");
    }

    FixedLengthStorage(
        int slot_size, const std::string& name, const Path& dest_dir = "./")
        : slot_size(slot_size),
          name(name),
          dest_dir(dest_dir),
          is_referenced(MaxPageInMemory, false),
          is_dirty(MaxPageInMemory, false),
          frames(MaxPageInMemory, nullptr),
          buffer_ptr(new Byte[buffer_size]()) {
            static_assert(MaxPageInMemory > 0, "MaxPageInMemory should be > 0 when using this constructor");
            auto indexFs = this->getIndexFs<this->in_mode>();
            indexFs.seekg(0, std::ios::end);
            if (indexFs.tellg() > 0) {
                // 文件非空: 从文件读取 page_num
                this->readPageNum();
            }
    }

    template <typename T>
    Rid Put(const T &data) {
        PagePtr non_full_page_ptr = nullptr;
        std::size_t frame_id = 0;
        // 尝试从内存中的缓存页中找到还没满的页面
        for (auto &pair : this->page_to_frame_map) {
            frame_id = pair.second;
            auto &cur_page_ptr = this->frames[frame_id];
            if (cur_page_ptr->isFull()) continue;
            // 找到了
            non_full_page_ptr = cur_page_ptr;
            break;
        }
        if (not non_full_page_ptr) {
            // 如果内存中的缓存页都满了: 这里就直接创建一个新的页面
            // 换出一个旧的, 得到 frame_id
            frame_id = this->swapPageOut();
            // 创建新的
            non_full_page_ptr = std::make_shared<Page>(this->page_num,
                                                       this->slot_size,
                                                       DataType,
                                                       this->getFrameAddr(frame_id),
                                                       this->page_size_in_k);
            this->initNewPage(non_full_page_ptr, frame_id);
            // 总页数增加
            ++this->page_num;
        }
        // 未满页的 page_id
        auto page_id = non_full_page_ptr->PageId();
        this->is_referenced[frame_id] = true;
        this->is_dirty[frame_id] = true;

        // 插入新的槽
        auto insert_result = non_full_page_ptr->insert();
        auto cur_slot = std::get<1>(insert_result);
        // 设置槽中数据
        cur_slot.Set(data);
        return std::get<0>(insert_result);
    }

    template <typename T>
    std::unique_ptr<T> Get(const Rid &rid) {
        if (not this->pageInMemory(rid.page_id)) {
            // 把想要的 rid.page_id 换入
            this->swapPageIn(rid.page_id, this->swapPageOut());
        }
        auto frame_id = this->page_to_frame_map[rid.page_id];
        this->is_referenced[frame_id] = true;
        // 得到想要的页面
        auto &cur_page_ptr = this->frames[frame_id];
        // 得到想要的槽
        auto cur_slot = cur_page_ptr->select(rid.slot_id);
        std::unique_ptr<T> ptr;
        // 复制出槽中数据
        cur_slot.Get(ptr);
        return std::move(ptr);
    }

    ~FixedLengthStorage() {
        // 一页一页写出
        for (auto &pair : this->page_to_frame_map) {
            auto page_id = pair.first;
            auto frame_id = pair.second;
            if (not this->is_dirty[frame_id]) continue;
            auto fs = this->getFs<this->out_mode>(page_id);
            this->writePageOut(frame_id, fs);
        }
        this->writePageNum();
    }

    int SlotSize() const {
        return this->slot_size;
    }
  private:

    bool pageInMemory(int page_id) const {
        return this->page_to_frame_map.find(page_id) != this->page_to_frame_map.end();
    }

    bool framesFull() const {
        return this->page_to_frame_map.size() == MaxPageInMemory;
    }

    void initNewPage(PagePtr new_page_ptr, std::size_t frame_id) {
        this->frames[frame_id] = new_page_ptr;
        this->page_to_frame_map.insert({ new_page_ptr->PageId(), frame_id });
    }

    /**
     * @description 尝试找到一个牺牲页 并把它换出去 得到空槽
     * @return 返回空槽id
     */
    std::size_t swapPageOut() {
        auto victim_page_id = this->findVictimPage();
        if (victim_page_id == -1) {
            // 没有牺牲页 说明还有得缓存 直接返回新的帧id
            return this->page_to_frame_map.size();
        }
        // 得到牺牲页的帧id 然后考虑把它换出去
        auto frame_id = this->page_to_frame_map.at(victim_page_id);
        if (this->is_dirty[frame_id]) {
            auto fs = this->getFs<this->out_mode>(victim_page_id);
            this->writePageOut(frame_id, fs);
        }
        this->frames[frame_id].reset();
        this->page_to_frame_map.erase(victim_page_id);
        return frame_id;
    }

    void writePageOut(std::size_t frame_id, std::ostream &out) {
        // 找到对应页　地址偏移一波　写回去
        auto page_ptr = this->frames[frame_id];
        auto page_position = this->begin_pos + page_ptr->PageId() * this->page_size;
        out.seekp(page_position);
        page_ptr->sync(out);
    }

    void swapPageIn(int page_in_id, std::size_t frame_id) {
        auto fs = this->getFs<this->in_mode>(page_in_id);
        return this->readPageIn(page_in_id, frame_id, fs);
    }

    void readPageIn(int page_in_id, std::size_t frame_id, std::istream &in) {
        // 找到对应页　地址偏移一波　读进来
        auto page_position = this->begin_pos + page_in_id * this->page_size;
        in.seekg(page_position);
        auto new_page_ptr = std::make_shared<Page>(page_in_id,
                                                   in,
                                                   this->getFrameAddr(frame_id),
                                                   this->page_size_in_k);
        // 构建各种关系
        this->initNewPage(new_page_ptr, frame_id);
        this->is_dirty[frame_id] = false;
        this->is_referenced[frame_id] = true;
    }

    /**
     * @description 如果换出后不马上换入，就会gg，作者太菜了
     * @return 牺牲页的 page_id
     */
    int findVictimPage() {
        static_assert(MaxPageInMemory > 0, "MaxPageInMemory should be > 0 when calling findVictimPage");
        // 没满 就没有页要牺牲
        if (not this->framesFull()) {
            return -1;
        }

        // 帧存满页 现在有一页需要牺牲
        // 循环 + 引用位　判断
        static int64_t victim_frame_id = 0;
        for (std::size_t index = 0; index < MaxPageInMemory; ++index) {
            if (not this->frames[victim_frame_id]) continue;
            if (this->is_referenced[victim_frame_id]) {
                // 如果引用位有效　关闭引用位
                this->is_referenced[victim_frame_id] = false;
            } else {
                return this->frames[victim_frame_id]->PageId();
            }

            victim_frame_id = (victim_frame_id + 1) % MaxPageInMemory;
        }
        if (this->frames[victim_frame_id]) {
            return this->frames[victim_frame_id]->PageId();
        }
        assert("didn't swap in right after swapping out?");
    }

    template <std::ios::openmode openmode>
    std::fstream getFs(int page_id) {
        std::stringstream ss;
        ss << "." << page_id;
        auto filename = this->dest_dir + this->name + ss.str();
        std::fstream fs(filename, openmode);
        return std::move(fs);
    }

    template <std::ios::openmode openmode>
    std::fstream getIndexFs() {
        auto filename = this->dest_dir + this->name + ".index";
        std::fstream fs(filename, openmode);
        return std::move(fs);
    }

    void readPageNum() {
        auto indexFs = this->getIndexFs<this->in_mode>();
        auto page_num_addr = reinterpret_cast<char *>(&this->page_num);
        constexpr auto page_num_size = sizeof(this->page_num);
        indexFs.seekg(0);
        indexFs.read(page_num_addr, page_num_size);
    }

    void writePageNum() {
        auto indexFs = this->getIndexFs<this->out_mode>();
        auto page_num_addr = reinterpret_cast<char *>(&this->page_num);
        constexpr auto page_num_size = sizeof(this->page_num);
        indexFs.seekp(0);
        indexFs.write(page_num_addr, page_num_size);
    }

    inline Byte *getFrameAddr(std::size_t frame_id) const {
        auto buffer_ptr = reinterpret_cast<Byte (*)[page_size]>(this->buffer_ptr.get());
        return reinterpret_cast<Byte *>(buffer_ptr + frame_id);
    }

  private:
    int slot_size;
    std::string name;
    Path dest_dir;
    int page_num = 0;

    BitSet is_referenced;
    BitSet is_dirty;
    std::vector<PagePtr> frames;
    std::unordered_map<int, std::size_t> page_to_frame_map;
    std::unique_ptr<Byte> buffer_ptr;
    std::fstream fs;

    static constexpr std::ios::openmode in_mode = std::ios::binary | std::ios::in;
    static constexpr std::ios::openmode out_mode = std::ios::binary | std::ios::out;
    static constexpr std::ios::openmode openmode = std::ios::binary | std::ios::in | std::ios::out;
    static constexpr std::size_t page_size_in_k = (BytesPerPage + 1023) / 1024;
    static constexpr std::size_t page_size = page_size_in_k * 1024;
    static constexpr std::size_t buffer_size = page_size * MaxPageInMemory;
    static constexpr std::size_t begin_pos = 0;
};

// 不知道为什么要加下面这一坨才能沃克
template <int64_t a, Rid::DataType b, int64_t c, bool d>
constexpr std::size_t FixedLengthStorage<a, b, c, d>::page_size_in_k;


class MemoryOnlyStorage;
class NormalStorage;

class RecordStorage {
  public:
    RecordStorage(const Path& dest_dir, int dimension = -1) {}
    RecordStorage() = default;
    RecordStorage(const RecordStorage&) = default;
    /**
     * stores record to the storage and returns the rid pointing to the
     * record, may first try to store in memory, then
     * flush to file if specified
     */
    virtual Rid Put(const Record&) {};

    /**
     * finds the record specified by rid
     * @return nullptr if not found
     */
    virtual std::unique_ptr<Record> Get(const Rid& rid) {};

    /**
     * dump all data to specific path,
     */
    virtual void DumpTo(const Path& dest_dir) {};

    virtual ~RecordStorage() {}
};
/**
 * storage store node
 */
class NodeStorage {
  public:
    NodeStorage(const Path& dest_dir, int dimension);
    std::unique_ptr<BallTreeNode> Get(Rid rid);
    Rid Put(const BallTreeNode& node);

    std::unique_ptr<BallTreeNode> GetRoot();
    Rid PutRoot(const BallTreeNode& node);

    inline int GetDimension() {
        return dimension;
    }
  private:
    int dimension;
    std::fstream others;
    std::unique_ptr<FixedLengthStorage<64, Rid::branch, 2>> branch_storage;
    std::unique_ptr<FixedLengthStorage<64, Rid::leaf, 4>> leaf_storage;
};

class NormalStorage: public RecordStorage {
    public:
    NormalStorage(const Path& dest_dir, int dimension);
    virtual Rid Put(const Record& record) override;
    virtual std::unique_ptr<Record> Get(const Rid& rid) override;
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

inline std::unique_ptr<RecordStorage> GetRecordStorage(Path& dest_dir, int dim) {
    return std::make_unique<RecordStorage>(dest_dir, dim);
}

inline std::unique_ptr<NodeStorage> GetNodeStorage(Path& dest_dir, int dim) {
    return std::make_unique<NodeStorage>(dest_dir, dim);
}


inline std::unique_ptr<SimpleStorage> GetSimpleStorage() {
    return std::make_unique<SimpleStorage>();
}

}  // namespace storage_factory
#endif
