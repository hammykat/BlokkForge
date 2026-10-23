#pragma once

#include <cstdint>
#include <xsimd/xsimd.hpp>
#include <SDL3/SDL.h>

namespace Blokk
{

template <class Arch>
void ProcessVelocities_SIMD(
    float* PosX,
    float* PosY,
    const float* VelX,
    const float* VelY,
    std::uint32_t Size
) {
    using Batch = xsimd::batch<float, Arch>;
    constexpr std::size_t Lanes = Batch::size;

    Uint64 delta = SDL_GetTicks();

    std::uint32_t i = 0;
    for (; i + Lanes <= Size; i += Lanes)
    {
        const Batch XPos = Batch::load_unaligned(PosX + i);
        const Batch YPos = Batch::load_unaligned(PosY + i);

        const Batch XVel = Batch::load_unaligned(VelX + i);
        const Batch YVel = Batch::load_unaligned(VelY + i);

        const Batch NewXPos = (XPos + XVel) * delta;
        const Batch NewYPos = (YPos + YVel) * delta;

        NewXPos.store_unaligned(PosX + i);
        NewYPos.store_unaligned(PosY + i);
    }

    // Remaining elements that don't fill a SIMD register.
    for (; i < Size; ++i)
    {
        PosX[i] += VelX[i];
        PosY[i] += VelY[i];
    }
}

}