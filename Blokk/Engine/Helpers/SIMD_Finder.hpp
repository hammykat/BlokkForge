#pragma once


#ifdef Blokk_Support_SIMD_ALL
#define Blokk_Support_SIMD_AVX512
#define Blokk_Support_SIMD_AVX2
#define Blokk_Support_SIMD_AVX
#define Blokk_Support_SIMD_SSE4_2
#define Blokk_Support_SIMD_SSE4_1
#define Blokk_Support_SIMD_SSSE3
#define Blokk_Support_SIMD_SSE3
#define Blokk_Support_SIMD_SSE2
#define Blokk_Support_SIMD_NEON
#define Blokk_Support_SIMD_NEON64
#define Blokk_Support_SIMD_I8MM
#define Blokk_Support_SIMD_SVE128
#define Blokk_Support_SIMD_SVE256
#define Blokk_Support_SIMD_SVE512
#define Blokk_Support_SIMD_RVV128
#define Blokk_Support_SIMD_RVV256
#define Blokk_Support_SIMD_RVV512
#define Blokk_Support_SIMD_VSX
#define Blokk_Support_SIMD_VXE
#define Blokk_Support_SIMD_AVX_VNNI
#define Blokk_Support_SIMD_AVX512IFMA
#define Blokk_Support_SIMD_AVX512VBMI
#define Blokk_Support_SIMD_AVX512VBMI2
#define Blokk_Support_SIMD_AVX512VNNI
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
        SSE3,
        SSSE3,
        SSE4_1,
        SSE4_2,

        AVX,
        AVX2,

        AVX512_128,
        AVX512_256,

        AVX512F,
        AVX512CD,
        AVX512DQ,
        AVX512BW,
        AVX512VL,

        AVX512IFMA,
        AVX512VBMI,
        AVX512VBMI2,
        AVX512VNNI,

        AVX_VNNI,

        // ARM
        NEON,
        NEON64,
        I8MM,

        // ARM SVE
        SVE128,
        SVE256,
        SVE512,

        // RISC-V
        RVV128,
        RVV256,
        RVV512,

        // POWER
        VSX,

        // IBM Z
        VXE,

        // WebAssembly
        WASM
    };


    inline SIMDLevel DetectSIMD()
    {
        /*
            This function is intended to be called once during initialization.

            xsimd performs the actual platform-specific hardware detection.

            available_architectures() only reports architectures which are:
                1. Supported by the current xsimd build/compiler
                2. Actually available at runtime

            Therefore, we never intentionally select an instruction set that
            xsimd says the current machine cannot execute.
        */

        const auto Available = xsimd::available_architectures();


        // ============================================================
        // x86

        if (Available.has(xsimd::avx512vnni< xsimd::avx512vbmi2 >{}))
            return SIMDLevel::AVX512VNNI;

        if (Available.has(xsimd::avx512vbmi2{}))
            return SIMDLevel::AVX512VBMI2;

        if (Available.has(xsimd::avx512vbmi{}))
            return SIMDLevel::AVX512VBMI;

        if (Available.has(xsimd::avx512ifma{}))
            return SIMDLevel::AVX512IFMA;

        if (Available.has(xsimd::avx512bw{}))
            return SIMDLevel::AVX512BW;

        if (Available.has(xsimd::avx512dq{}))
            return SIMDLevel::AVX512DQ;

        if (Available.has(xsimd::avx512vl{}))
            return SIMDLevel::AVX512VL;

        if (Available.has(xsimd::avx512cd{}))
            return SIMDLevel::AVX512CD;

        if (Available.has(xsimd::avx512f{}))
            return SIMDLevel::AVX512F;

        if (Available.has(xsimd::avx512vl_256{}))
            return SIMDLevel::AVX512_256;

        if (Available.has(xsimd::avx512vl_128{}))
            return SIMDLevel::AVX512_128;

        if (Available.has(xsimd::avxvnni{}))
            return SIMDLevel::AVX_VNNI;

        if (Available.has(xsimd::avx2{}))
            return SIMDLevel::AVX2;

        if (Available.has(xsimd::avx{}))
            return SIMDLevel::AVX;

        if (Available.has(xsimd::sse4_2{}))
            return SIMDLevel::SSE4_2;

        if (Available.has(xsimd::sse4_1{}))
            return SIMDLevel::SSE4_1;

        if (Available.has(xsimd::ssse3{}))
            return SIMDLevel::SSSE3;

        if (Available.has(xsimd::sse3{}))
            return SIMDLevel::SSE3;

        if (Available.has(xsimd::sse2{}))
            return SIMDLevel::SSE2;


        // ARM / NEON ========================================

        if (Available.has(xsimd::i8mm<xsimd::neon64>{}))
            return SIMDLevel::I8MM;

        if (Available.has(xsimd::neon64{}))
            return SIMDLevel::NEON64;

        if (Available.has(xsimd::neon{}))
            return SIMDLevel::NEON;


        // ============================================================
        // ARM SVE
        //
        // Check largest vector length first.

        if (Available.has(xsimd::detail::sve<512>{}))
            return SIMDLevel::SVE512;

        if (Available.has(xsimd::detail::sve<256>{}))
            return SIMDLevel::SVE256;

        if (Available.has(xsimd::detail::sve<128>{}))
            return SIMDLevel::SVE128;


        // ============================================================
        // RISC-V Vector

        if (Available.has(xsimd::detail::rvv<512>{}))
            return SIMDLevel::RVV512;

        if (Available.has(xsimd::detail::rvv<256>{}))
            return SIMDLevel::RVV256;

        if (Available.has(xsimd::detail::rvv<128>{}))
            return SIMDLevel::RVV128;


        // POWER
        
        if (Available.has(xsimd::vsx{}))
            return SIMDLevel::VSX;


        // IBM Z

        if (Available.has(xsimd::vxe{}))
            return SIMDLevel::VXE;


        // WebAssembly

        if (Available.has(xsimd::wasm{}))
            return SIMDLevel::WASM;


        // Nothing usable

        return SIMDLevel::Scalar;
    }


    /*
        Optional cached version.

        This makes absolutely sure the expensive detection happens only once.
    */
    inline SIMDLevel GetSIMD()
    {
        static const SIMDLevel Level = DetectSIMD();

        return Level;
    }

} // namespace Blokk