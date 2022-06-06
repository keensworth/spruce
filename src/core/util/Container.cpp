#include "Container.h"

namespace spr {

template<typename T>
Container<T>::Container(){
    m_data = std::vector<T>(512);
    m_writeIndices = std::vector<T>(64);
}

template<typename T>
Container<T>::Container(int size){
    m_data = std::vector<T>(size);
    m_writeIndices = std::vector<T>(64);
}

template<typename T>
int Container<T>::getSize(){
    return m_data.size();
}

template<typename T>
int Container<T>::add(T data){
    if (m_writeIndices.size() == 0){
        m_data.push_back(data);
        m_lastWriteIndex = m_data.size()-1;
    } else {
        int index = m_writeIndices.pop_back();
        m_data.at(index) = data;
        m_lastWriteIndex = index;
    }
    return m_lastWriteIndex;
}

template<typename T>
int Container<T>::set(int index, T data){
    m_data.at(index) = data;
}

template<typename T>
void Container<T>::remove(int index){
    if (index >= m_data.size()){
        return;
    }

    if (std::count(m_writeIndices.begin(), m_writeIndices.end(), index)) {
        return;
    }

    m_writeIndices.push_back(index);
}

template<typename T>
T Container<T>::get(int index){
    return m_data.at(index);
}

template<typename T>
void Container<T>::clear(){
    m_data.clear();
}

}