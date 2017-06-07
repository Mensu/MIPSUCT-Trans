#include "page.h"

Page::Page(int page_id, IntType slot_size, Rid::DataType type, Byte* pool_base,
           IntType page_size_in_k = 64)
          : Page(page_id, page_size_in_k, pool_base) {
    m_slot_size = slot_size;
    Byte* solt_size_addr = m_slot_pool + page_size - sizeof(IntType);
    *reinterpret_cast<IntType*>(solt_size_addr) = m_slot_size;
    *reinterpret_cast<Rid::DataType*>(solt_size_addr - sizeof(Rid::DataType)) = type;
    this->type = type;
    init();
    m_slot_map = std::vector<bool>(m_bitmap_size * 8, false);
}

Page::Page(int page_id, std::istream& in, Byte* pool_base, int page_size_in_k = 64)
          : Page(page_id, page_size_in_k, pool_base) {
    in.read(reinterpret_cast<char*>(m_slot_pool), page_size);
    Byte* solt_size_addr = m_slot_pool + page_size - sizeof(IntType);
    m_slot_size = *reinterpret_cast<IntType*>(solt_size_addr);
    type = *reinterpret_cast<Rid::DataType*>(solt_size_addr - sizeof(Rid::DataType));
    init();
    initBitMap();
}

Page::Page(int page_id, IntType page_size_in_k, Byte* pool_base)
          : m_page_id(page_id),
            m_slot_num(0),
            m_dirty(false),
            m_slot_size(0),
            page_size(page_size_in_k * 1024 / sizeof(Byte)),
            m_slot_pool(pool_base) {}
/**
 * @Description Write data to file;
 */
void Page::sync(std::ostream& out) {
    restoreBitMap();
    out.write(reinterpret_cast<char*>(m_slot_pool), page_size);
}

/**
 * @Description Select a slot according to solt id.
 */
Slot Page::select(const int& slot_id) {
    assert(slot_id < m_slot_map.size() and m_slot_map[slot_id]);
    return makeSlot(slot_id);
}

/**
 * @Description Insert new slot in page
 * @Return tuple of Rid and Slot.
 */
std::tuple<Rid, Slot> Page::insert() {
    assert(not isFull());
    Rid ret(m_page_id, m_total_slot, type);
    for (int i = 0; i < m_total_slot; ++i) {
        if (m_slot_map[i] == false) {
            m_slot_map[i] = true;
            m_slot_num++;
            ret.slot_id = i;
            break;
        }
    }
    this->m_dirty = true;
    return std::make_tuple(ret, makeSlot(ret.slot_id));
}

/**
 * @Description Drop a slot from page, just reset its bit in bitmap
 */
bool Page::drop(const int& slot_id) {
    if (slot_id < m_total_slot || m_slot_map[slot_id] == false) return false;
    m_slot_map[slot_id] = false;
    m_slot_num--;
    m_dirty = true;
    return true;
}

void Page::initBitMap() {
    m_slot_map.clear();
    m_slot_map.reserve(m_bitmap_size * 8);
    for (IntType i = 0; i < m_bitmap_size; ++i) {
        for (int bit_pos = 0; bit_pos < 8; ++bit_pos) {
            bool valid = bitmap_pos[i] & (1 << bit_pos);
            if (valid) {
                m_slot_num++;
            }
            m_slot_map.push_back(valid);
        }
    }
}

void Page::restoreBitMap() {
    if (not m_dirty) return;
    Byte* end_pos = m_slot_pool + page_size - sizeof(IntType);
    auto it = m_slot_map.begin();
    for (Byte* pos = bitmap_pos; pos != end_pos; ++pos) {
        *pos = bitsToByte(it, it + 8);
        // dangerous step !!!
        it += 8;
    }
    m_dirty = false;
}

void Page::init() {
    // reserve 8 bits for bitmap alignemt
    const int page_bit =
            (page_size - sizeof(IntType) - sizeof(Rid::DataType) - 1) * 8;
    // 1 bit for bitmap
    const int bit_per_slot = m_slot_size * 8 + 1;
    m_total_slot = page_bit / bit_per_slot;
    m_bitmap_size = m_total_slot / 8 + 1;
    bitmap_pos = m_slot_pool + m_slot_size * m_total_slot;
}

