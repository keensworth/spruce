#include "MaskNode.h"

namespace spr {

template <typename T, typename U, typename V>
MaskNode<T,U,V>::MaskNode(){
    m_height = 8;
    m_nodeData = new std::array<MaskNode>(16);
    m_initialized = true;
}
template <typename T, typename U, typename V>
MaskNode<T,U,V>::MaskNode(int height){\
    m_height = height;
    if (m_height > 0){
        m_nodeData = new std::array<MaskNode>(16);
    } else {
        m_leafData = V;
    }
}


//add
template <typename T, typename U, typename V>
void MaskNode<T,U,V>::add(T key, U data){
    int currIndex;
    MaskNode* currNode = this;

    for (int height = m_height; height > 0; height--){
        currIndex = subIndex(key, height);

        if (!currNode->getBranch(currIndex).m_initialized){
            currNode->buldBranch(currIndex, height);
            currNode->setBit(currIndex,1);
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(key, currNode->getHeight());
    currNode.addLeafData(currIndex, data);
}

//remove
template <typename T, typename U, typename V>
void MaskNode<T,U,V>::remove(T key){
    int currIndex;
    MaskNode* currNode = this;

    for (int height = m_height; height > 0; height--){
        currIndex = subIndex(key, height);

        if (!currNode->getBranch(currIndex).m_initialized){
            return;
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(key, currNode->getHeight());
    currNode.removeLeafData(currIndex);
}

//get
template <typename T, typename U, typename V>
V MaskNode<T,U,V>::get(T key){
    int currIndex;
    MaskNode* currNode = this;

    for (int height = m_height; height > 0; height--){
        currIndex = subIndex(key, height);

        if (!currNode->getBranch(currIndex).m_initialized){
            return NULL;
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(key, currNode->getHeight());
    return currNode.getLeafData(currIndex);
}

template <typename T, typename U, typename V>
void append(std::vector<V>& v1, std::vector<V>& v2){
    v1.insert(std::end(v1), std::begin(v2), std::end(v2));
}

//get_accum
template <typename T, typename U, typename V>
std::vector<V> MaskNode<T,U,V>::getAccum(T key){
    int currIndex = subIndex(key, m_height);
    MaskNode* currNode = this;
    std::vector<V> accum;

    for (int i = 0; i < 16; i++){
        if (m_height > 0){
            if (!((currIndex & i) == currIndex && getBranch(i)->m_initialized)){
                continue;
            }
            append(accum, getBranch(currIndex).getAccum(key));
        } else {
            append(accum, getLeafData(key));
        }
    } 

    return accum;
}


template <typename T, typename U, typename V>
void MaskNode<T,U,V>::buildBranch(int branchIndex, int height){
    m_nodeData.at(branchIndex) = new MaskNode(height-1);
}

template <typename T, typename U, typename V>
MaskNode<T,U,V>* MaskNode<T,U,V>::getBranch(int branchIndex){
    return m_nodeData.at(branchIndex);
}

template <typename T, typename U, typename V>
void MaskNode<T,U,V>::setBranch(int branchIndex, MaskNode<T,U,V> branchData){
    m_nodeData.at(branchIndex) = branchData;
}

template <typename T, typename U, typename V>
int MaskNode<T,U,V>::subIndex(T num, int height){
    return ((num>>>(height*4))&0b1111);
}

template <typename T, typename U, typename V>
void MaskNode<T,U,V>::setBit(int bitIndex, int bitValue){
    int setterMask = (byte)((0b00000001)<<bitIndex);
    if (bitValue==0){
        m_mask &= (setterMask^0xffffffff);
    } else {
        m_mask |= setterMask;
    }
}

template <typename T, typename U, typename V>
int MaskNode<T,U,V>::getBit(int bitIndex){
    return (m_mask>>>bitIndex)&0b1;
}
}