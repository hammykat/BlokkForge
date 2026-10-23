#pragma once

#include <cstdint>
#include <vector>

#include <xsimd/xsimd.hpp>

namespace Blokk
{

template <class Arch>
std::vector<std::uint32_t> CheckVisible_SIMD(
    std::int32_t ScreenWidth,
    std::int32_t ScreenHeight,
    const std::int32_t* AnimWidths,
    const std::int32_t* AnimHeights,
    const float* XPositions,
    const float* YPositions,
    std::uint32_t Count,
    std::uint32_t Start
#ifdef Blokk_CamEnabled
    , float CameraX,
    float CameraY
#endif
) {
    using FloatBatch = xsimd::batch<float, Arch>;
    using IntBatch = xsimd::batch<std::int32_t, Arch>;

    std::vector<std::uint32_t> Visible;
    Visible.reserve(Count);

    const std::size_t Lanes = FloatBatch::size;

    const FloatBatch ScreenW(
        static_cast<float>(ScreenWidth)
    );

    const FloatBatch ScreenH(
        static_cast<float>(ScreenHeight)
    );

#ifdef Blokk_CamEnabled
    const FloatBatch CamX(CameraX);
    const FloatBatch CamY(CameraY);
#endif

    std::uint32_t i = 0;

    for (; i + Lanes <= Count;
         i += static_cast<std::uint32_t>(Lanes))
    {
        const FloatBatch X =
            xsimd::load_unaligned(XPositions + i);

        const FloatBatch Y =
            xsimd::load_unaligned(YPositions + i);

        const IntBatch WidthInt =
            xsimd::load_unaligned(AnimWidths + i);

        const IntBatch HeightInt =
            xsimd::load_unaligned(AnimHeights + i);

        const FloatBatch Width =
            xsimd::to_float(WidthInt);

        const FloatBatch Height =
            xsimd::to_float(HeightInt);

#ifdef Blokk_CamEnabled
        const FloatBatch Left = X - CamX;
        const FloatBatch Top = Y - CamY;
#else
        const FloatBatch Left = X;
        const FloatBatch Top = Y;
#endif

        const FloatBatch Right =
            Left + Width;

        const FloatBatch Bottom =
            Top + Height;

        const auto VisibleMask =
            (Right > FloatBatch(0.0f)) &
            (Left < ScreenW) &
            (Bottom > FloatBatch(0.0f)) &
            (Top < ScreenH);

        for (std::size_t Lane = 0;
             Lane < Lanes;
             ++Lane)
        {
            if (VisibleMask.get(Lane))
            {
                Visible.push_back(
                    Start + i + Lane
                );
            }
        }
    }

    // Scalar remainder
    for (; i < Count; ++i)
    {
#ifdef Blokk_CamEnabled
        const float X =
            XPositions[i] - CameraX;

        const float Y =
            YPositions[i] - CameraY;
#else
        const float X = XPositions[i];
        const float Y = YPositions[i];
#endif

        const float Right =
            X + static_cast<float>(AnimWidths[i]);

        const float Bottom =
            Y + static_cast<float>(AnimHeights[i]);

        if (Right > 0.0f &&
            X < static_cast<float>(ScreenWidth) &&
            Bottom > 0.0f &&
            Y < static_cast<float>(ScreenHeight))
        {
            Visible.push_back(Start + i);
        }
    }

    return Visible;
}

}
