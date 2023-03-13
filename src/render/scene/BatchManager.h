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

    uint32 getDrawCount();

    void reset();
    void destroy();
    
private:
    BatchNode m_batches;
    uint32 m_drawCount;

    void addDraw(DrawData draw, Batch batchInfo);
    void getDrawData(TempBuffer<DrawData>& result);

    friend class SceneManager;
};
}