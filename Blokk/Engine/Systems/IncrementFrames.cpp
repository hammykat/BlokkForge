#ifdef Blokk_Rendering_Enabled

#include <cstdint>

#include <xsimd/xsimd.hpp>

#include "EngineClassData.hpp"
#include "IncrementFrames-inl.h"

namespace Blokk
{

void EngineCore::IncrementFrames()
{
    const std::uint32_t Size =
        static_cast<std::uint32_t>(FrameNums.size());

    auto Process = [&]<typename Arch>()
    {
        IncrementFramesImpl<Arch>(FrameNums.data(), Size);
    };

#ifdef Blokk_Support_SIMD_AVX512VNNI
    if (xsimd::avx512vnni<xsimd::avx512vbmi2>::available())
    {
        Process.template operator()<xsimd::avx512vnni<xsimd::avx512vbmi2>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX512VBMI2
    if (xsimd::avx512vbmi2::available())
    {
        Process.template operator()<xsimd::avx512vbmi2>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX512VBMI
    if (xsimd::avx512vbmi::available())
    {
        Process.template operator()<xsimd::avx512vbmi>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX512IFMA
    if (xsimd::avx512ifma::available())
    {
        Process.template operator()<xsimd::avx512ifma>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX512
    if (xsimd::avx512bw::available())
    {
        Process.template operator()<xsimd::avx512bw>();
        return;
    }
    if (xsimd::avx512dq::available())
    {
        Process.template operator()<xsimd::avx512dq>();
        return;
    }
    if (xsimd::avx512vl::available())
    {
        Process.template operator()<xsimd::avx512vl>();
        return;
    }
    if (xsimd::avx512cd::available())
    {
        Process.template operator()<xsimd::avx512cd>();
        return;
    }
    if (xsimd::avx512f::available())
    {
        Process.template operator()<xsimd::avx512f>();
        return;
    }
    if (xsimd::avx512vl_256::available())
    {
        Process.template operator()<xsimd::avx512vl_256>();
        return;
    }
    if (xsimd::avx512vl_128::available())
    {
        Process.template operator()<xsimd::avx512vl_128>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX_VNNI
    if (xsimd::avxvnni::available())
    {
        Process.template operator()<xsimd::avxvnni>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX2
    if (xsimd::avx2::available())
    {
        Process.template operator()<xsimd::avx2>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_AVX
    if (xsimd::avx::available())
    {
        Process.template operator()<xsimd::avx>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSE4_2
    if (xsimd::sse4_2::available())
    {
        Process.template operator()<xsimd::sse4_2>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSE4_1
    if (xsimd::sse4_1::available())
    {
        Process.template operator()<xsimd::sse4_1>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSSE3
    if (xsimd::ssse3::available())
    {
        Process.template operator()<xsimd::ssse3>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSE3
    if (xsimd::sse3::available())
    {
        Process.template operator()<xsimd::sse3>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SSE2
    if (xsimd::sse2::available())
    {
        Process.template operator()<xsimd::sse2>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_I8MM
    if (xsimd::i8mm<xsimd::neon64>::available())
    {
        Process.template operator()<xsimd::i8mm<xsimd::neon64>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_NEON64
    if (xsimd::neon64::available())
    {
        Process.template operator()<xsimd::neon64>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_NEON
    if (xsimd::neon::available())
    {
        Process.template operator()<xsimd::neon>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SVE512
    if (xsimd::detail::sve<512>::available())
    {
        Process.template operator()<xsimd::detail::sve<512>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SVE256
    if (xsimd::detail::sve<256>::available())
    {
        Process.template operator()<xsimd::detail::sve<256>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_SVE128
    if (xsimd::detail::sve<128>::available())
    {
        Process.template operator()<xsimd::detail::sve<128>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_RVV512
    if (xsimd::detail::rvv<512>::available())
    {
        Process.template operator()<xsimd::detail::rvv<512>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_RVV256
    if (xsimd::detail::rvv<256>::available())
    {
        Process.template operator()<xsimd::detail::rvv<256>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_RVV128
    if (xsimd::detail::rvv<128>::available())
    {
        Process.template operator()<xsimd::detail::rvv<128>>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_VSX
    if (xsimd::vsx::available())
    {
        Process.template operator()<xsimd::vsx>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_VXE
    if (xsimd::vxe::available())
    {
        Process.template operator()<xsimd::vxe>();
        return;
    }
#endif
#ifdef Blokk_Support_SIMD_WASM
    if (xsimd::wasm::available())
    {
        Process.template operator()<xsimd::wasm>();
        return;
    }
#endif

    for (std::uint32_t i = 0; i < Size; ++i)
    {
        ++FrameNums[i];
    }
}

}

#endif
