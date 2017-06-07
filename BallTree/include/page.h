#ifndef _PAGE_H
#define _PAGE_H

#include <cassert>
#include <fstream>
#include <vector>
#include "rid.h"
#include "slot.h"
#include "Utility.h"
#include "BallTreeImpl.h"

/**
 * page entity in memory form. Extract the data in page with type T.
 */

class Page {

  public:
    const int page_size;
    using Pool = Byte*;
    using IntType = std::size_t;
    using BitMap = std::vector<bool>;

  public:
    /**
     * @Description Create a new page
     */
    Page(int page_id, IntType slot_size, Rid::DataType type, Byte* pool_base, IntType page_size_in_k);

    /**
     * @Description Make a page from binary page file
     */
    Page(int page_id, std::istream& in, Byte* pool_base, int page_size_in_k);
    /**
     * @Description Write data to file;
     */
    void sync(std::ostream& out);

    /**
     * @Description Select a slot according to solt id.
     */
    Slot select(const int& slot_id);

    /**
     * @Description Insert new slot in page
     * @Return tuple of Rid and Slot.
     */
    std::tuple<Rid, Slot> insert();

    /**
     * @Description Drop a slot from page, just reset its bit in bitmap
     */
    bool drop(const int& slot_id);

    static size_t GetSize(Rid::DataType type, int dimension);

    inline bool isFull() { return m_slot_num >= m_total_slot; }

    inline int PageId() const {
      return this->m_page_id;
    }

  private:
   /**
    * @Description Build Bitmap from buffer memory.
    */
    void initBitMap();

    /**
     * @ Restore bitmap to buffer memory if necessary.
     */
    void restoreBitMap();

    /**
     * @Description the delegate construction
     * @Param delegate_constructor unused.
     */
    Page(int page_id, IntType page_size_in_k, Byte* pool_base);
    void init();

    inline Slot makeSlot(int slot_id) {
        return Slot(m_slot_pool + m_slot_size * slot_id, m_slot_size, type);
    }

 private:
    int m_page_id;
    bool m_dirty;

    Pool m_slot_pool;
    BitMap m_slot_map;
    Byte* bitmap_pos;

    int m_total_slot;
    int m_slot_num;
    IntType m_slot_size;
    IntType m_bitmap_size;
    Rid::DataType type;
};

#endif