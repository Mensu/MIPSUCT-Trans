#ifndef _PAGE_H
#define _PAGE_H

#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>
#include "record.h"
#include "rid.h"
#include "Utility.h"

template <typename T>
class Page {
  public:
    const int page_size;
    using Pool = std::unique_ptr<Byte>;
    using IntType = std::uint64_t;
    using BitMap = std::vector<bool>;
    using DataPointer = std::unique_ptr<std::vector<T>>;
  public:
    Page(int page_id, Pool& pool, int slot_size = -1, int page_size_in_k = 64)
            : m_slot_pool(std::move(pool)),
              m_page_id(page_id),
              m_slot_num(0),
              m_dirty(false),
              m_slot_size(slot_size),
              page_size(page_size_in_k * 1024 / sizeof(Byte)) {
        Byte* pool_addr = m_slot_pool.get();
        if (m_slot_size == -1) {
            m_slot_size = *reinterpret_cast<IntType*>(pool_addr + page_size - sizeof(IntType));
        } else {
            memset(pool_addr, 0, page_size);
            *reinterpret_cast<IntType*>(pool_addr + page_size - sizeof(IntType)) = m_slot_size;
        }
        
        // reserve 8 bits for bitmap alignemt
        const int page_bit = (page_size - sizeof(IntType) - 1) * 8;
        // 1 bit for bitmap
        const int bit_per_slot = m_slot_size * 8 + 1;
        m_total_slot = page_bit / bit_per_slot;
        m_bitmap_size = m_total_slot / 8 + 1;
        bitmap_pos = pool_addr + page_size - m_bitmap_size - sizeof(uint64_t);
        if (slot_size == -1) {
            this->initBitMap();
        } else {
            m_slot_map = std::vector<bool>(m_bitmap_size * 8, false);
        }
    }

    const Byte* getData() {
        restoreBitMap();
        return this->m_slot_pool.get();
    }


    DataPointer read(const int& slot_id) {
        assert(slot_id < m_slot_map.size() and m_slot_map[slot_id]);
        Byte* slot_data_pos = m_slot_pool.get() + m_slot_size * slot_id;
        Byte* slot_end_pos = slot_data_pos + m_slot_size;
        assert((slot_end_pos - slot_data_pos) % sizeof(T) == 0);
        std::vector<T> data_set(reinterpret_cast<T*>(slot_data_pos),
                                    reinterpret_cast<T*>(slot_end_pos));
        return std::make_unique<std::vector<T>> (std::move(data_set));
    }

    void update(int slot_id, const std::vector<T>& data) {
        assert(slot_id < m_total_slot);
        if (m_slot_map[slot_id] == false) {
            m_slot_map[slot_id] = true;
            m_slot_num++;
        }
        assert(data.size() * sizeof(T) <= m_slot_size);
        Byte* slot_pos = m_slot_pool.get() + m_slot_size * slot_id;
        Byte* slot_end_pos = slot_pos + m_slot_size;
        T* data_iter = reinterpret_cast<T*>(slot_pos);
        for (const auto& item : data) {
            *data_iter = item;
            data_iter += 1;
        }
        m_dirty = true;
    }
    void update(int slot_id, const DataPointer& data_ptr) {
        return this->update(slot_id, *data_ptr);
    }

    /**
     * @Description Make a new record in page
     */
    Rid make(const std::vector<T>& data) {
        assert(!isFull());
        Rid ret(m_page_id, m_total_slot);
        for (int i = 0; i < m_total_slot; ++i) {
            if (m_slot_map[i] == false) {
                ret.slot_id = i;
                break;
            }
        }
        update(ret.slot_id, data);
        return ret;
    }

    Rid make(const DataPointer& data_ptr) {
        return this->make(*data_ptr);
    }

    bool drop(const int& slot_id) {
        if (slot_id < m_total_slot || m_slot_map[slot_id] == false) return false;
        m_slot_map[slot_id] = false;
        m_dirty = true;
        return true;
    }

    bool isFull() { return m_slot_num >= m_total_slot; }

  private:
    void initBitMap() {
        m_slot_map.clear();
        m_slot_map.reserve(m_bitmap_size * 8);
        for (int i = 0; i < m_bitmap_size; ++i) {
            for (int bit_pos = 0; bit_pos < 8; ++bit_pos) {
                bool valid = bitmap_pos[i] & (1 << bit_pos);
                if (valid) {
                    m_slot_num++;
                }
                m_slot_map.push_back(valid);
            }
        }
    }

    void restoreBitMap() {
        if (!m_dirty) return;
        Byte *end_pos = m_slot_pool.get() + page_size - sizeof(uint64_t);
        auto it = m_slot_map.begin();
        for (Byte *pos = bitmap_pos; pos != end_pos; ++pos) {
            *pos = bitsToByte(it, it + 8);
            // dangerous step !!!
            it += 8;
        }
        m_dirty = false;
    }

 private:
    int m_page_id;
    bool m_dirty;

    Pool m_slot_pool;
    BitMap m_slot_map;
    Byte* bitmap_pos;

    int m_total_slot;
    int m_slot_num;
    uint64_t m_slot_size;
    uint64_t m_bitmap_size;
};

#endif