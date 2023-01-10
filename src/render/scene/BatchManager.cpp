#include "BatchManager.h"
#include "BatchNode.h"

namespace spr::gfx {

void BatchManager::addDraw(DrawData draw, uint32 meshId, uint32 materialFlags){
    m_batches.add(materialFlags, meshId, draw);
}

std::vector<Batch> BatchManager::getBatches(uint32 materialFlags){
    return m_batches.getAny(materialFlags);
}

void BatchManager::prepareDraws(){
    //m_batches.uploadDrawData(rm);
}

void BatchManager::reset(){
    // rm->deleteDraws
    m_batches.~BatchNode();
    m_batches = BatchNode();
}

void BatchManager::destroy(){
    // rm->deleteDraws
    m_batches.~BatchNode();
}

}