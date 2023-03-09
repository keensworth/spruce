#pragma once

#include <vector>
#include "spruce_core.h"
#include "BatchNode.h"
#include "Material.h"
#include "Draw.h"

namespace spr::gfx {

class BatchManager {

public:
    BatchManager(){
        m_batches = BatchNode();
    }
    ~BatchManager(){}

    void addDraw(DrawData draw, Batch batchInfo);
    void getDrawBatches(BatchMaterialQuery query, std::vector<DrawBatch>& result);
    void reset();
    void destroy();
    
private:
    BatchNode m_batches;
};
}