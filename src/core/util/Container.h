#pragma once

#include <algorithm> 
#include <vector>
#include <iostream>


namespace spr {

template <typename T>
class Container {
public:

    Container(){
        m_data = std::vector<T>();
        m_writeIndices = std::vector<int>();
    }

    Container(int size){
        m_data = std::vector<T>();
        m_writeIndices = std::vector<int>();
    }

    ~Container(){}

    int getSize(){
        return m_data.size();
    }

    // add data to container
    int add(T data){
        if (m_writeIndices.size() == 0){
            m_data.push_back(data);
            lastWriteIndex = m_data.size()-1;
        } else {
            int index = m_writeIndices.back();
            m_writeIndices.pop_back();
            m_data.at(index) = data;
            lastWriteIndex = index;
        }
        return lastWriteIndex;
    }

    // set data at index
    int set(int index, T data){
        m_data.at(index) = data;
        return index;
    }

    // get data at index
    T get(int index){
        return m_data.at(index);
    }

    // remove data and allow overwrite
    void remove(int index, bool eraseData){
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
    void erase(T data){
        auto itr = std::find(m_data.begin(), m_data.end(), data);
        if(itr == m_data.end())
            return;
        auto index = std::distance(m_data.begin(), itr);

        if (index >= m_data.size()){
            return;
        }

        m_data.erase(m_data.begin() + index);

        int toErase = -1;
        for (int& i : m_writeIndices){
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
        int dataSize = m_data.size();
        m_data.insert(std::end(m_data), std::begin(c2.m_data), std::end(c2.m_data));
        for (int i = 0; i < c2.m_writeIndices.size(); i++){
            c2.m_writeIndices.at(i) += dataSize;
        }
        m_writeIndices.insert(std::end(m_writeIndices), std::begin(c2.m_writeIndices), std::end(c2.m_writeIndices));
    }

    void clear(){
        m_data.clear();
    }
    
    // last write index
    int lastWriteIndex;

private:
    std::vector<T> m_data;
    std::vector<int> m_writeIndices;
};

}