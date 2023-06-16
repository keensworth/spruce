#pragma once

#include <algorithm> 
#include <vector>
#include <iostream>

#include "../spruce_core.h"


namespace spr {

template <typename T>
class Container {
public:

    Container(){
        m_data = std::vector<T>();
        m_writeIndices = std::vector<int32>();
    }

    Container(int32 size){
        m_data = std::vector<T>();
        m_writeIndices = std::vector<int32>();
    }

    ~Container(){}

    int32 getSize(){
        return m_data.size();
    }

    // add data to container
    int32 add(T data){
        if (m_writeIndices.size() == 0){
            m_data.push_back(data);
            lastWriteIndex = m_data.size()-1;
        } else {
            int32 index = m_writeIndices.back();
            m_writeIndices.pop_back();
            m_data.at(index) = data;
            lastWriteIndex = index;
        }
        return lastWriteIndex;
    }

    // set data at index
    int32 set(int32 index, T data){
        m_data.at(index) = data;
        return index;
    }

    // get data at index
    T& get(int32 index){
        return m_data.at(index);
    }

    T* data(){
        return m_data.data();
    }

    std::vector<T>& vector(){
        return m_data;
    }

    // remove data and allow overwrite
    void remove(int32 index, bool eraseData){
        if (index >= m_data.size()){
            return;
        }

        m_writeIndices.push_back(index);

        if (eraseData){
            m_data.at(index) = nullptr;
        }
    }

    // remove data and allow overwrite
    void remove(T data){
        auto itr = std::find(m_data.begin(), m_data.end(), data);
        if(itr == m_data.end())
            return;
        auto index = std::distance(m_data.begin(), itr);

        if (index >= m_data.size()){
            return;
        }

        m_writeIndices.push_back(index);
    }

    // erase data, shrink underlying container
    void eraseData(T data){
        auto itr = std::find(m_data.begin(), m_data.end(), data);
        if(itr == m_data.end())
            return;
        auto index = std::distance(m_data.begin(), itr);

        if (index >= m_data.size()){
            return;
        }

        m_data.erase(m_data.begin() + index);

        int32 toErase = -1;
        for (int32& i : m_writeIndices){
            if (i == index){
                toErase = index;
            }
            if (i > index){
                i--;
            }
        }
        if (toErase != -1){
            auto itrErase = std::find(m_writeIndices.begin(), m_writeIndices.end(), toErase);
            if(itrErase == m_writeIndices.end())
                return;
            auto indexErase = std::distance(m_writeIndices.begin(), itrErase);

            m_writeIndices.erase(m_writeIndices.begin()+indexErase);
        }
    }

    // erase data, shrink underlying container
    void erase(uint32 index){
        m_data.erase(m_data.begin() + index);

        int32 toErase = -1;
        for (int32& i : m_writeIndices){
            if (i == index){
                toErase = index;
            }
            if (i > index){
                i--;
            }
        }
        if (toErase != -1){
            auto itrErase = std::find(m_writeIndices.begin(), m_writeIndices.end(), toErase);
            if(itrErase == m_writeIndices.end())
                return;
            auto indexErase = std::distance(m_writeIndices.begin(), itrErase);

            m_writeIndices.erase(m_writeIndices.begin()+indexErase);
        }
    }

    // append container to end of current
    void append(Container<T>& c2){
        int32 dataSize = m_data.size();
        m_data.insert(std::end(m_data), std::begin(c2.m_data), std::end(c2.m_data));
        for (int32 i = 0; i < c2.m_writeIndices.size(); i++){
            c2.m_writeIndices.at(i) += dataSize;
        }
        m_writeIndices.insert(std::end(m_writeIndices), std::begin(c2.m_writeIndices), std::end(c2.m_writeIndices));
    }

    void clear(){
        m_data.clear();
    }
    
    // last write index
    int32 lastWriteIndex;

private:
    std::vector<T> m_data;
    std::vector<int32> m_writeIndices;
};

}