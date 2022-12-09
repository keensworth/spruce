#pragma once

namespace spr::gfx{
class RenderPassRenderer{
public:
    RenderPassRenderer(){}
    ~RenderPassRenderer(){}

    void beginRecording();
    void submit();

private:

};
}