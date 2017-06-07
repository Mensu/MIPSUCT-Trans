#ifndef __RID_H
#define __RID_H

#include <cstdint>
struct Rid {
    using DataType = std::uint8_t;
    static constexpr DataType record = 0;
    static constexpr DataType branch = 1;
    static constexpr DataType leaf = 2;
    Rid(int page_id, int slot_id, DataType type = Rid::record)
        : page_id(page_id), slot_id(slot_id), type(type) {}
    int page_id, slot_id;
    DataType type;
};

#endif
