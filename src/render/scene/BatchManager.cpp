#include "BatchManager.h"
#include "BatchNode.h"
#include "Draw.h"
#include "Material.h"

namespace spr::gfx {

BatchManager::BatchManager(){
    
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

uint32 BatchManager::getQuadVertexOffset(){
    return m_quadVertexOffset;
}

void BatchManager::setQuadInfo(Batch quadBatch, uint32 quadVertexOffset){
    m_quadBatch = quadBatch;
    m_quadVertexOffset = quadVertexOffset;
}

Batch BatchManager::getCubeBatch(){
    return m_cubeBatch;
}

uint32 BatchManager::getCubeVertexOffset(){
    return m_cubeVertexOffset;
}

void BatchManager::setCubeInfo(Batch cubeBatch, uint32 cubeVertexOffset){
    m_cubeBatch = cubeBatch;
    m_cubeVertexOffset = cubeVertexOffset;
}

uint32 BatchManager::getDrawCount(){
    return m_drawCount;
}

void BatchManager::reset(){
    m_batches = BatchNode();
    m_drawCount = 0;
}

void BatchManager::destroy(){
}

}