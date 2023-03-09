#include "BatchManager.h"
#include "BatchNode.h"

namespace spr::gfx {

void BatchManager::addDraw(DrawData draw, Batch batchInfo){
    m_batches.add(draw, batchInfo);
}

void BatchManager::getDrawBatches(BatchMaterialQuery query, std::vector<DrawBatch>& result){
    m_batches.get(query, result);
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