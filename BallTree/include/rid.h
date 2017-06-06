#ifndef __RID_H
#define __RID_H

struct Rid {
    Rid(int page_id, int slot_id) : page_id(page_id), slot_id(slot_id) {}
    int page_id, slot_id;
};

#endif
