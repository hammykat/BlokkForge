#include "EngineClassData.hpp"

#ifdef Blokk_Rendering_Enabled
#if (Blokk_Rendering_CullType == 0)

#include <cstdint>
#include <vector>

#include <xsimd/xsimd.hpp>

#include "Visibility_BasicCulling-inl.h"

namespace Blokk
{

void EngineCore::CheckVisibilityFn_Basic(
    Worker& worker
) {
    IndexRange Range = worker.getRange();

#ifdef Blokk_CamEnabled
    const float CameraX =
        static_cast<float>(Camera.GetXPosition());

    const float CameraY =
        static_cast<float>(Camera.GetYPosition());
#endif

    const std::int32_t* Widths = &AnimWidths[Range.Start];
    const std::int32_t* Heights = &AnimHeights[Range.Start];
    const float* XPos = &XPositions[Range.Start];
    const float* YPos = &YPositions[Range.Start];
    const std::uint32_t Size = Range.GetSize();

    auto Process = [&]<typename Arch>()
    {
        return CheckVisible_SIMD<Arch>(
            ScreenWidth,
            ScreenHeight,
            Widths,
            Heights,
            XPos,
            YPos,
            Size,
            Range.Start
#ifdef Blokk_CamEnabled
            , CameraX,
            CameraY
#endif
        );
    };

#ifdef Blokk_Support_SIMD_AVX512VNNI
    if (xsimd::avx512vnni<xsimd::avx512vbmi2>::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512vnni<xsimd::avx512vbmi2>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX512VBMI2
    if (xsimd::avx512vbmi2::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512vbmi2>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX512VBMI
    if (xsimd::avx512vbmi::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512vbmi>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX512IFMA
    if (xsimd::avx512ifma::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512ifma>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX512
    if (xsimd::avx512bw::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512bw>();
        return;
    }
    if (xsimd::avx512dq::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512dq>();
        return;
    }
    if (xsimd::avx512vl::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512vl>();
        return;
    }
    if (xsimd::avx512cd::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512cd>();
        return;
    }
    if (xsimd::avx512f::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512f>();
        return;
    }
    if (xsimd::avx512vl_256::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512vl_256>();
        return;
    }
    if (xsimd::avx512vl_128::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx512vl_128>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX_VNNI
    if (xsimd::avxvnni::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avxvnni>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX2
    if (xsimd::avx2::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx2>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX
    if (xsimd::avx::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::avx>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSE4_2
    if (xsimd::sse4_2::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::sse4_2>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSE4_1
    if (xsimd::sse4_1::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::sse4_1>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSSE3
    if (xsimd::ssse3::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::ssse3>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSE3
    if (xsimd::sse3::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::sse3>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSE2
    if (xsimd::sse2::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::sse2>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_I8MM
    if (xsimd::i8mm<xsimd::neon64>::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::i8mm<xsimd::neon64>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_NEON64
    if (xsimd::neon64::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::neon64>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_NEON
    if (xsimd::neon::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::neon>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SVE512
    if (xsimd::detail::sve<512>::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::detail::sve<512>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SVE256
    if (xsimd::detail::sve<256>::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::detail::sve<256>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SVE128
    if (xsimd::detail::sve<128>::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::detail::sve<128>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_RVV512
    if (xsimd::detail::rvv<512>::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::detail::rvv<512>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_RVV256
    if (xsimd::detail::rvv<256>::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::detail::rvv<256>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_RVV128
    if (xsimd::detail::rvv<128>::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::detail::rvv<128>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_VSX
    if (xsimd::vsx::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::vsx>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_VXE
    if (xsimd::vxe::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::vxe>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_WASM
    if (xsimd::wasm::available())
    {
        Thread->IdxResult = Process.template operator()<xsimd::wasm>();
        return;
    }
#endif

    Thread->IdxResult.clear();
    Thread->IdxResult.reserve(Size);
    for (std::uint32_t i = 0; i < Size; ++i)
    {
#ifdef Blokk_CamEnabled
        const float X = XPos[i] - CameraX;
        const float Y = YPos[i] - CameraY;
#else
        const float X = XPos[i];
        const float Y = YPos[i];
#endif
        const float Right = X + static_cast<float>(Widths[i]);
        const float Bottom = Y + static_cast<float>(Heights[i]);

        if (Right > 0.0f &&
            X < static_cast<float>(ScreenWidth) &&
            Bottom > 0.0f &&
            Y < static_cast<float>(ScreenHeight))
        {
            Thread->IdxResult.push_back(Range.Start + i);
        }
    }
}

}

#endif // Blokk_Rendering_CullType == 0

#endif // Blokk_Rendering_Enabled
