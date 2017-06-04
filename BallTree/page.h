#ifndef _PAGE_H
#define _PAGE_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <vector>
#include "record.h"
#include "rid.h"

class Page {
  public:
    const int page_size;
    using Pool = std::unique_ptr<char>;
    using BitMap = std::vector<bool>;

  public:
    Page(int page_id, Pool& pool, int slot_size = -1, int page_size = 64)
            : m_slot_pool(std::move(pool)),
                m_page_id(page_id),
                m_slot_num(0),
                m_dirty(false),
                m_slot_size(slot_size),
                page_size(page_size * 1024 / sizeof(char)) {
        char* pool_addr = m_slot_pool.get();
        if (m_slot_size == -1) {
            m_slot_size = *reinterpret_cast<int*>(pool_addr + page_size - sizeof(int));
        } else {
            memset(pool_addr, 0, page_size);
            *reinterpret_cast<int*>(pool_addr) = m_slot_size;
        }
        assert(m_slot_size % 8 == 0);
        // reserve 8 bits for bitmap alignemt
        const int page_bit = (page_size - sizeof(uint64_t) - 1) * 8;
        // 1 bit for bitmap
        const int bit_per_slot = m_slot_size * 8 + 1;
        m_total_slot = page_bit / bit_per_slot + 1;
        m_bitmap_size = m_total_slot / 8 + 1;
        if (slot_size != -1) {
            this->initBitMap(pool_addr + page_size - m_bitmap_size -
                                             sizeof(uint64_t));
        } else {
            m_slot_map = std::vector<bool>(m_bitmap_size, false);
        }
    }

    const char* getData() {
        restoreBitMap();
        return this->m_slot_pool.get();
    }

    std::unique_ptr<Record> read(const int& slot_id) {
        assert(slot_id < m_slot_map.size() and m_slot_map[slot_id]);
        char* slot_pos = m_slot_pool.get() + m_slot_size * slot_id;
        char* slot_end_pos = slot_pos + m_slot_size;
        int* index = reinterpret_cast<int*>(slot_pos);
        char* slot_data_pos = slot_pos + sizeof(int);
        assert((slot_end_pos - slot_data_pos) % sizeof(float) == 0);
        std::vector<float> data_set(reinterpret_cast<float*>(slot_data_pos),
                                    reinterpret_cast<float*>(slot_end_pos));
        return std::make_unique<Record>(*index, std::move(data_set));
    }

    bool update(int slot_id, const Record& record) {
        assert(slot_id < m_total_slot);
        if (m_slot_map[slot_id] == false) {
            m_slot_map[slot_id] = true;
            m_slot_num++;
        }
        assert(record.Size() * sizeof(float) <= m_slot_size + sizeof(int));
        char* slot_pos = m_slot_pool.get() + m_slot_size * slot_id;
        char* slot_end_pos = slot_pos + m_slot_size;
        *reinterpret_cast<int*>(slot_pos) = record.index;
        float* data_iter = reinterpret_cast<float*>(slot_pos + sizeof(int));
        for (const auto& data : record.data) {
            *data_iter = data;
            data_iter += 1;
        }
        m_dirty = true;
    }
    /**
     * @Description Make a new record in page
     */
    Rid make(const Record& record) {
        assert(!isFull());
        Rid ret(m_page_id, m_total_slot);
        for (int i = 0; i < m_total_slot; ++i) {
            if (m_slot_map[i] == false) {
                ret.slot_id = i;
                break;
            }
        }
        update(ret.slot_id, record);
        return ret;
    }

    bool isFull() { return m_slot_num < m_total_slot; }

  private:
    void initBitMap(const char* start_pos) {
        m_slot_map.clear();
        m_slot_map.reserve(m_bitmap_size * 8);
        for (int i = 0; i < m_bitmap_size; ++i) {
            for (int bit_pos = 0; bit_pos < 8; ++bit_pos) {
                bool valid = start_pos[i] & (1 << bit_pos);
                if (valid) {
                    m_slot_num++;
                }
                m_slot_map.push_back(valid);
            }
        }
    }

    void restoreBitMap() {}

 private:
    int m_page_id;
    bool m_dirty;

    Pool m_slot_pool;
    BitMap m_slot_map;

    int m_total_slot;
    int m_slot_num;
    uint64_t m_slot_size;
    uint64_t m_bitmap_size;
};

#endif