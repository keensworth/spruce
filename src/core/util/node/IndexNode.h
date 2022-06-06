#pragma once

#include <vector>

namespace spr {

class IndexNode {
public:
    IndexNode();
    IndexNode(int height);
    ~IndexNode() {
        for (int i = 0; i < m_nodeData.size(); i++){
            delete m_nodeData.at(i);
        }
    }

    //add
    void add(int key, int data);
    //remove
    void remove(int key);
    //get
    int get(int key);
    //get_accum

    int getHeight(){
        return m_height;
    }

private:
    int m_height;
    int m_mask;
    std::vector<IndexNode*> m_nodeData;
    std::vector<int> m_leafData;
    bool m_initialized;

    void addLeafData(int key, int data);
    void removeLeafData(int key);
    int getLeafData(int key);

    void buildBranch(int branchIndex, int height);
    IndexNode* getBranch(int branchIndex);
    void setBranch(int branchIndex, IndexNode* branchData);

    int subIndex(int num, int height);
    void setBit(int bitIndex, int bitValue);
    int getBit(int bitIndex);
};

}