#pragma once

#include <vector>
#include <array>

namespace spr {

template <typename T, typename U, typename V>
class MaskNode {
public:
    MaskNode();
    MaskNode(int height);
    ~MaskNode() {
        delete m_nodeData;
    }

    //add
    void add(T key, U data);
    //remove
    void remove(T key);
    //get
    V get(T key);
    //get_accum
    std::vector<V> getAccum(T key);

    int getHeight(){
        return m_height;
    }

private:
    T m_height;
    int m_mask;
    std::array<MaskNode*> m_nodeData;
    V m_leafData;
    bool m_initialized;

    virtual void addLeafData(int key, U data);
    virtual void removeLeafData(int key);
    virtual V getLeafData(int key);
    virtual std::vector<V> getAccumLeafData(int key);

    void buildBranch(int branchIndex, int height);
    MaskNode* getBranch(int branchIndex);
    void setBranch(int branchIndex, MaskNode branchData);

    int subIndex(T num, int height);
    void setBit(int bitIndex, int bitValue);
    int getBit(int bitIndex);
};

}