#pragma once

#ifdef Blokk_Support_SIMD_ALL
#define Blokk_Support_SIMD_AVX512
#define Blokk_Support_SIMD_AVX2
#define Blokk_Support_SIMD_AVX
#define Blokk_Support_SIMD_SSE2
#define Blokk_Support_SIMD_NEON
#define Blokk_Support_SIMD_SVE
#define Blokk_Support_SIMD_RVV
#define Blokk_Support_SIMD_VSX
#define Blokk_Support_SIMD_WASM
#define Blokk_Support_SIMD_Scalar
#endif

#include <xsimd/xsimd.hpp>

namespace Blokk
{

    enum class SIMDLevel
    {
        Scalar,

        // x86
        SSE2,
        AVX,
        AVX2,
        AVX512,

        // ARM
        NEON,
        SVE,

        // RISC-V
        RVV,

        // PowerPC
        VSX,

        // WebAssembly
        WASM
    };


    inline SIMDLevel DetectSIMD()
    {
        const auto Available = xsimd::available_architectures();

        // x86
        if (Available.has(xsimd::avx512f{}))
            return SIMDLevel::AVX512;

        if (Available.has(xsimd::avx2{}))
            return SIMDLevel::AVX2;

        if (Available.has(xsimd::avx{}))
            return SIMDLevel::AVX;

        if (Available.has(xsimd::sse2{}))
            return SIMDLevel::SSE2;


        // ARM
        if (Available.has(xsimd::neon64{}))
            return SIMDLevel::NEON;

        if (Available.has(xsimd::neon{}))
            return SIMDLevel::NEON;


        // ARM SVE
        if (Available.has(xsimd::detail::sve<512>{}) ||
            Available.has(xsimd::detail::sve<256>{}) ||
            Available.has(xsimd::detail::sve<128>{}))
            return SIMDLevel::SVE;


        // RISC-V
        if (Available.has(xsimd::detail::rvv<512>{}) ||
            Available.has(xsimd::detail::rvv<256>{}) ||
            Available.has(xsimd::detail::rvv<128>{}))
            return SIMDLevel::RVV;


        // PowerPC
        if (Available.has(xsimd::vsx{}))
            return SIMDLevel::VSX;


        // WebAssembly
        if (Available.has(xsimd::wasm{}))
            return SIMDLevel::WASM;


        return SIMDLevel::Scalar;
    }


    inline SIMDLevel GetSIMD()
    {
        static const SIMDLevel Level = DetectSIMD();
        return Level;
    }

}