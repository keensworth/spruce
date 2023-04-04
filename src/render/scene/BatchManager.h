#pragma once

#include <vector>
#include "spruce_core.h"
#include "BatchNode.h"
#include "Material.h"
#include "Draw.h"

namespace spr::gfx {

class BatchManager {

public:
    BatchManager();
    ~BatchManager();

    void getBatches(MaterialQuery query, std::vector<Batch>& result);
    Batch getQuadBatch();
    uint32 getQuadVertexOffset();
    uint32 getDrawCount();

    void reset();
    void destroy();
    
private:
    BatchNode m_batches;
    uint32 m_drawCount;

    Batch m_quadBatch;
    uint32 m_quadVertexOffset;

    void addDraw(DrawData draw, Batch batchInfo);
    void getDrawData(TempBuffer<DrawData>& result);
    void setQuadInfo(Batch quadBatch, uint32 quadVertexOffset);

    friend class SceneManager;
};
}