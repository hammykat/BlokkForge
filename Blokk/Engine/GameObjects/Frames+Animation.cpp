#include "GameObject.hpp"

#ifdef Blokk_Rendering_Enabled

namespace Blokk {

void GameObject::SetFrameNum(uint32_t FrameNum)
{
    // Set val
    Engine->FrameNums[EngineIdx] = FrameNum;
}

void GameObject::SetAnimation(string AnimName)
{
    auto It = Engine->AnimNames.find(AnimName);

    if (It == Engine->AnimNames.end())
        return;

    // Set val
    Engine->AnimNums[EngineIdx] = It->second;
}

}

#endif