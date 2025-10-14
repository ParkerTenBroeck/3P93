//
// Created by may on 05/10/25.
//

#ifndef SLICE_H
#define SLICE_H
#include "iter.h"
#include "types.h"

template <typename T>
class slice {
    T* m_data;
    usize m_len;
    slice(ptr_mut<T> data, usize len) : m_data(data), m_len(len) {}
public:

    [[nodiscard]]
    static constexpr slice from_raw(ptr_mut<T> data, usize len) {
        return slice(data, len);
    }

    [[nodiscard]]
    static constexpr slice from_ref(T& data) {
        return slice(&data, 1);
    }

    [[nodiscard]]
    constexpr usize len() const { return m_len; }

    [[nodiscard]]
    constexpr ptr<T> data() const {
        return m_data;
    }

    [[nodiscard]]
    constexpr ptr_mut<T> data() {
        return m_data;
    }

    [[nodiscard]]
    constexpr ref<T> operator[](usize index) const {
        return m_data[index];
    }

    [[nodiscard]]
    constexpr ref_mut<T> operator[](usize index) {
        return m_data[index];
    }

    [[nodiscard]]
    constexpr SliceIter<const T> iter() const {
        return SliceIter<T>(m_data, m_data+m_len);
    }

    [[nodiscard]]
    constexpr SliceIter<T> iter() {
        return SliceIter<T>(m_data, m_data+m_len);
    }
};

#endif //SLICE_H
