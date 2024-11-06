#pragma once

#include <initializer_list>
#include <vector>
#include <array>
#include "spruce_core.h"
#include "debug/SprLog.h"

namespace spr {

template <typename T>
class TempBuffer;

template <typename T>
class Span {
public:
    Span() :  m_data(nullptr), m_size(0) {}
    Span(T* data, uint32 size) : m_data(data), m_size(size) {}
    Span(const T* data, uint32 size) : m_data(data), m_size(size) {}
    Span(std::initializer_list<T> ilist) : m_data(ilist.begin()), m_size(ilist.size()) {}
    Span(std::vector<T>& vector) : m_data(vector.data()), m_size(vector.size()) {}
    template <std::size_t N>
    Span(std::array<T, N>& array) : m_data(array.data()), m_size(N) {}
    template <std::size_t N>
    Span(const T (&data)[N]) : m_data(data), m_size(N) {}
    Span(TempBuffer<T>& buffer) : m_data(buffer.getData()), m_size(buffer.getSize()) {}

    T operator[](uint32 index) {
        if (index >= m_size)
            SprLog::error({{"[Span] ", color::GRADIENT17}, {"[operator[]] index "}, {index}, {" out of range for span size of "}, {m_size}});
        
        return m_data[index];
    }

    const T& operator[](uint32 index) const {
        if (index >= m_size)
            SprLog::error({{"[Span] ", color::GRADIENT17}, {"[const operator[]] index "}, {index}, {" out of range for span size of "}, {m_size}});
        
        return m_data[index];
    }

    const T* data() const {
        return m_data;
    }

    const T* begin() const {
        return m_data;
    }

    const T* end() const {
        return m_data + m_size;
    }

    uint32 size() const {
        return m_size;
    }

    std::vector<T> toVec() {
        std::vector<T> out(m_size);
        out.insert(out.begin(), begin(), end());
        return out;
    }

private:
    const T* m_data;
    uint32 m_size;
};

}