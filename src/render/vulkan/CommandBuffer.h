#pragma once

namespace spr::gfx{
class CommandBuffer{
public:
    CommandBuffer(){}
    ~CommandBuffer(){}

    void beginRecording();
    void submit();

private:

};
}