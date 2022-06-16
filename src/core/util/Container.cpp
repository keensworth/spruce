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
        lastWriteIndex = m_data.size()-1;
    } else {
        int index = m_writeIndices.pop_back();
        m_data.at(index) = data;
        lastWriteIndex = index;
    }
    return lastWriteIndex;
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

    if (std::count(m_writeIndices.begin(), m_writeIndices.end(), index) == 0) {
        return;
    }

    m_writeIndices.push_back(index);
}

template<typename T>
void Container<T>::remove(T data){
    auto itr = std::find(m_data.begin(), m_data.end(), data);
    if(itr == m_data.end())
        return;
    auto index = std::distance(m_data.begin(), itr);

    if (index >= m_data.size()){
        return;
    }

    if (std::count(m_writeIndices.begin(), m_writeIndices.end(), index) == 0) {
        return;
    }

    m_writeIndices.push_back(index);
}

// append container to end of current
template<typename T>
void Container<T>::append(Container<T>& c2){
    m_data.insert(std::end(m_data), std::begin(c2.m_data), std::end(c2.m_data));

    for (int i = 0; i < c2.m_writeIndices.size(); i++){
        c2.m_writeIndices.at(i) += m_data.size();
    }

    m_writeIndices.insert(std::end(m_writeIndices), std::begin(c2.m_writeIndices), std::end(c2.m_writeIndices));
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