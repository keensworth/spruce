#pragma once

#include <vector>

namespace spr {

template <typename T>
class Container {
public:
    Container();
    Container(int size);
    ~Container() {}

    // get size of container
    int getSize();

    // add data to container
    int add(T data);

    // set data at given index
    int set(int index, T data);

    // get data at index
    T get(int index);

    // clear data at given index
    void remove(int index);

    // clear container
    void clear();

    // last write index
    int lastWriteIndex;

private:
    std::vector<T> m_data;
    std::vector<T> m_writeIndices;
};

}