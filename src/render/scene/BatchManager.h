#pragma once

#include <vector>
#include "spruce_core.h"
#include "BatchNode.h"

namespace spr::gfx {

class BatchManager {

public:
    BatchManager();
    ~BatchManager();

    void getBatches(MaterialQuery query, std::vector<Batch>& result);
    Batch getQuadBatch();
    uint32 getQuadVertexOffset();
    Batch getCubeBatch();
    uint32 getCubeVertexOffset();
    uint32 getDrawCount();

    void reset();
    void destroy();
    
private:
    BatchNode m_batches;
    uint32 m_drawCount;

    Batch m_quadBatch;
    uint32 m_quadVertexOffset;

    Batch m_cubeBatch;
    uint32 m_cubeVertexOffset;

    void addDraw(DrawData draw, Batch batchInfo);
    void getDrawData(TempBuffer<DrawData>& result);
    void setQuadInfo(Batch quadBatch, uint32 quadVertexOffset);
    void setCubeInfo(Batch cubeBatch, uint32 cubeVertexOffset);

    friend class SceneManager;
};
}