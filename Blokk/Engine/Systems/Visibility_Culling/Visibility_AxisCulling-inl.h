#pragma once

#include <cstdint>
#include <vector>

#include <xsimd/xsimd.hpp>

namespace Blokk
{

// Axis Y
// -------------------------------------------------------------------------

template <class Arch>
std::vector<std::uint32_t> CheckVisible_AxisY_SIMD(
    std::uint32_t ScreenHeight,
    const std::uint32_t* AnimHeights,
    const float* PosY,
    std::uint32_t Size,
    std::uint32_t StartIdx

#ifdef Blokk_CamEnabled
    , float CameraY
#endif
)
{
    using FloatBatch = xsimd::batch<float, Arch>;
    using UIntBatch = xsimd::batch<std::uint32_t, Arch>;

    std::vector<std::uint32_t> ResultIdx;
    ResultIdx.reserve(Size);

    const std::size_t Lanes = FloatBatch::size;

    const FloatBatch Zero(0.0f);

    const FloatBatch ScrHeight(
        static_cast<float>(ScreenHeight)
    );

#ifdef Blokk_CamEnabled
    const FloatBatch CamY(CameraY);
#endif

    std::uint32_t i = 0;

    for (; i + Lanes <= Size;
         i += static_cast<std::uint32_t>(Lanes))
    {
        // Heights
        const UIntBatch IntHeights =
            xsimd::load_unaligned(AnimHeights + i);

        const FloatBatch Heights =
            xsimd::to_float(IntHeights);

        // Minimum and maximum Y
        FloatBatch MinYPos =
            xsimd::load_unaligned(PosY + i);

        FloatBatch MaxYPos =
            MinYPos + Heights;

        // Camera offset
#ifdef Blokk_CamEnabled
        MinYPos -= CamY;
        MaxYPos -= CamY;
#endif

        // Check visibility
        const auto YResult =
            (MinYPos < ScrHeight) &
            (MaxYPos > Zero);

        // Read the mask
        for (std::size_t Lane = 0;
             Lane < Lanes;
             ++Lane)
        {
            if (YResult.get(Lane))
            {
                ResultIdx.push_back(
                    StartIdx + i + Lane
                );
            }
        }
    }

    // Remaining objects
    for (; i < Size; ++i)
    {
        float YMin = PosY[i];
        float YMax =
            PosY[i] + static_cast<float>(AnimHeights[i]);

#ifdef Blokk_CamEnabled
        YMin -= CameraY;
        YMax -= CameraY;
#endif

        if (YMax > 0.0f &&
            YMin < static_cast<float>(ScreenHeight))
        {
            ResultIdx.push_back(StartIdx + i);
        }
    }

    return ResultIdx;
}


// Both Axis
// -------------------------------------------------------------------------

template <class Arch>
std::vector<std::uint32_t> CheckVisible_Axis_SIMD(
    std::uint32_t ScreenWidth,
    std::uint32_t ScreenHeight,

    const std::uint32_t* AnimWidths,
    const std::uint32_t* AnimHeights,

    const float* PosX,
    const float* PosY,

    std::uint32_t Size,
    std::uint32_t StartIdx

#ifdef Blokk_CamEnabled
    , float CameraX,
    float CameraY
#endif
)
{
    // First cull against Y.
    std::vector<std::uint32_t> YAxisCulled =
        CheckVisible_AxisY_SIMD<Arch>(
            ScreenHeight,
            AnimHeights,
            PosY,
            Size,
            StartIdx

#ifdef Blokk_CamEnabled
            , CameraY
#endif
        );

    // Then cull the surviving objects against X.
    std::vector<std::uint32_t> Result;
    Result.reserve(YAxisCulled.size());

    for (std::uint32_t Idx : YAxisCulled)
    {
        float MinX = PosX[Idx];

        float MaxX =
            MinX + static_cast<float>(AnimWidths[Idx]);

#ifdef Blokk_CamEnabled
        MinX -= CameraX;
        MaxX -= CameraX;
#endif

        if (MaxX > 0.0f &&
            MinX < static_cast<float>(ScreenWidth))
        {
            Result.push_back(Idx);
        }
    }

    return Result;
}

}
