#ifndef __RECORD_H
#define __RECORD_H

#include <vector>

struct Record {
    Record(int index, std::vector<float>&& data)
        : index(index), data(std::move(data)) {}

    Record(Record&&) = default;
    Record(const Record&) = default;
    Record& operator=(Record&&) = default;
    Record& operator=(const Record&) = default;
    ~Record() = default;

    size_t Size() const {
        return data.size();
    }

    int index;
    std::vector<float> data;
};

#endif
