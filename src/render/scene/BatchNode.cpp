#include <cmath>
#include "BatchNode.h"


namespace spr::gfx {

BatchNode::BatchNode(){
    m_height = 8;
    m_nodeData = std::vector<BatchNode*>(16);
    m_initialized = true;
    m_mask = 0;
}
BatchNode::BatchNode(uint32 height){
    m_height = height;
    m_initialized = true;
    m_mask = 0;
    if (m_height > 0){
        m_nodeData = std::vector<BatchNode*>(16);
    } else {
        m_leafData = std::vector<ska::flat_hash_map<uint32, DrawBatch>>(16);
    }
}


void BatchNode::add(DrawData draw, Batch batchInfo){
    uint32 currIndex;
    BatchNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(batchInfo.materialFlags, height);

        if (!branchInitialized(currIndex)){
            currNode->buildBranch(currIndex, height);
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(batchInfo.materialFlags, currNode->getHeight());
    currNode->addLeafData(currIndex, draw, batchInfo);
}


void BatchNode::remove(uint32 materialFlags){
    uint32 currIndex;
    BatchNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(materialFlags, height);

        if (!branchInitialized(currIndex)){
            return;
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(materialFlags, currNode->getHeight());
    currNode->removeLeafData(currIndex);
}


void BatchNode::getBatches(MaterialQuery query, std::vector<Batch>& result){
    QueryType queryType = {
        .usesHasAll     = (query.hasAll != 0),
        .usesHasAny     = (query.hasAny != 0),
        .usesHasExactly = (query.hasExactly != 0),
        .usesExcludes   = (query.excludes != 0),
        .foundAny = false
    };

    getBatchesRec(query, result, queryType);
}


void BatchNode::getBatchesRec(MaterialQuery query, std::vector<Batch>& result, QueryType queryType){
    uint32 hasAllMask     = subIndex(query.hasAll, m_height);
    uint32 hasAnyMask     = subIndex(query.hasAny, m_height);
    uint32 hasExactlyMask = subIndex(query.hasExactly, m_height);
    uint32 excludesMask   = subIndex(query.excludes, m_height);

    // check each branch for query validity
    for(uint32 branchMask = 0; branchMask < 16; branchMask++){
        if (!branchInitialized(branchMask))
                continue;
        // check conditions for each query type
        bool hasAllValid     = (hasAllMask & branchMask) == hasAllMask;
        bool hasAnyValid     = (hasAnyMask & branchMask) != 0;
        bool hasExactlyValid = (hasExactlyMask == branchMask);
        bool excludesValid   = (excludesMask & branchMask) == 0;

        // verify that every query being used is satisfied
        if (queryType.usesHasExactly && !hasExactlyValid)
            continue;
        if (queryType.usesHasAll && !hasAllValid)
            continue;
        if (queryType.usesExcludes && !excludesValid)
            continue;
        if (queryType.usesHasAny && hasAnyValid)
            queryType.foundAny = true;

        // relevant queries are satisfied, progress down tree
        // or grab draw batches from leaves
        if(m_height > 0){
            BatchNode* node = getBranch(branchMask);
            node->getBatchesRec(query, result, queryType);
        } else {
            if (queryType.usesHasAny && !queryType.foundAny)
                continue;
            getLeafBatches(branchMask, result);
        }
    }
}


void BatchNode::getDraws(MaterialQuery query, TempBuffer<DrawData>& result){
    QueryType queryType = {
        .usesHasAll     = (query.hasAll != 0),
        .usesHasAny     = (query.hasAny != 0),
        .usesHasExactly = (query.hasExactly != 0),
        .usesExcludes   = (query.excludes != 0),
        .foundAny = false
    };

    getDrawsRec(query, result, queryType);
}


void BatchNode::getDrawsRec(MaterialQuery query, TempBuffer<DrawData>& result, QueryType queryType){
    uint32 hasAllMask     = subIndex(query.hasAll, m_height);
    uint32 hasAnyMask     = subIndex(query.hasAny, m_height);
    uint32 hasExactlyMask = subIndex(query.hasExactly, m_height);
    uint32 excludesMask   = subIndex(query.excludes, m_height);

    // check each branch for query validity
    for(uint32 branchMask = 0; branchMask < 16; branchMask++){
        if (!branchInitialized(branchMask))
                continue;
        // check conditions for each query type
        bool hasAllValid     = (hasAllMask & branchMask) == hasAllMask;
        bool hasAnyValid     = (hasAnyMask & branchMask) != 0;
        bool hasExactlyValid = (hasExactlyMask == branchMask);
        bool excludesValid   = (excludesMask & branchMask) == 0;

        // verify that every query being used is satisfied
        if (queryType.usesHasExactly && !hasExactlyValid)
            continue;
        if (queryType.usesHasAll && !hasAllValid)
            continue;
        if (queryType.usesExcludes && !excludesValid)
            continue;
        if (queryType.usesHasAny && hasAnyValid)
            queryType.foundAny = true;

        // relevant queries are satisfied, progress down tree
        // or grab draw batches from leaves
        if(m_height > 0){
            BatchNode* node = getBranch(branchMask);
            node->getDrawsRec(query, result, queryType);
        } else {
            if (queryType.usesHasAny && !queryType.foundAny)
                continue;
            getLeafDraws(branchMask, result);
        }
    }
}


void BatchNode::addLeafData(uint32 branchIndex, DrawData draw, Batch batchInfo){
    ska::flat_hash_map<uint32, DrawBatch>& leafNode = m_leafData.at(branchIndex);

    // insert a new drawbatch if none exists for this material/mesh pair
    if (!leafNode.count(batchInfo.meshId)) {
        leafNode[batchInfo.meshId] = DrawBatch();
        leafNode[batchInfo.meshId].batch = batchInfo;
    } else {
        leafNode[batchInfo.meshId].batch.drawCount++;
    }

    // add draw to existing draw batch 
    leafNode[batchInfo.meshId].draws.push_back(draw);
}

void BatchNode::removeLeafData(uint32 branchIndex){
    m_leafData.at(branchIndex) = ska::flat_hash_map<uint32, DrawBatch>();
}

void BatchNode::getLeafBatches(uint32 branchIndex, std::vector<Batch>& dst){
    // iterate over every materialFlags/DrawBatch pair
    // at the given branch and store batch in dst
    for(auto item : m_leafData.at(branchIndex))
        dst.push_back(item.second.batch);
}

void BatchNode::getLeafDraws(uint32 branchIndex, TempBuffer<DrawData>& dst){
    // iterate over every materialFlags/DrawBatch pair
    // at the given branch and store every set of draws
    // in dst, storing the offset in the batch
    for(auto item : m_leafData.at(branchIndex)){
        DrawBatch& drawBatch = item.second;
        uint32 drawDataOffset = dst.insert(drawBatch.draws.data(), drawBatch.draws.size());
        drawBatch.batch.drawDataOffset = drawDataOffset;
    }
}


bool BatchNode::branchInitialized(uint32 branchIndex){
    return ((0b1 << branchIndex) & m_mask);
}

void BatchNode::buildBranch(uint32 branchIndex, uint32 height){
    setBit(branchIndex,1);
    m_nodeData.at(branchIndex) = new BatchNode(height-1);
}

BatchNode* BatchNode::getBranch(uint32 branchIndex){
    return m_nodeData.at(branchIndex);
}

void BatchNode::setBranch(uint32 branchIndex, BatchNode* branchData){
    setBit(branchIndex,1);
    m_nodeData.at(branchIndex) = branchData;
}


uint32 BatchNode::subIndex(uint32 num, uint32 height){
    return ((num>>(height*4))&0b1111);
}

void BatchNode::setBit(uint32 bitIndex, uint32 bitValue){
    uint32 setterMask = ((0b00000001)<<bitIndex);
    if (bitValue==0){
        m_mask &= (setterMask^0xffffffff);
    } else {
        m_mask |= setterMask;
    }
}

uint32 BatchNode::getBit(uint32 bitIndex){
    return (m_mask>>bitIndex)&0b1;
}
}