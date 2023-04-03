#include "BatchManager.h"
#include "BatchNode.h"
#include "Draw.h"

namespace spr::gfx {

BatchManager::BatchManager(){
    m_batches = BatchNode();
}

BatchManager::~BatchManager(){
    
}

void BatchManager::addDraw(DrawData draw, Batch batchInfo){
    m_batches.add(draw, batchInfo);
    m_drawCount++;
}

void BatchManager::getDrawData(TempBuffer<DrawData>& result){
    m_batches.getDraws({.excludes = MTL_NONE}, result);
}

void BatchManager::getBatches(MaterialQuery query, std::vector<Batch>& result){
    m_batches.getBatches(query, result);
}

Batch BatchManager::getQuadBatch(){
    return m_quadBatch;
}

uint32 BatchManager::getDrawCount(){
    return m_drawCount;
}

void BatchManager::setQuadBatch(Batch quadBatch){
    m_quadBatch = quadBatch;
}

void BatchManager::reset(){
    m_batches.~BatchNode();
    m_batches = BatchNode();
    m_drawCount = 0;
}

void BatchManager::destroy(){
    m_batches.~BatchNode();
}

}