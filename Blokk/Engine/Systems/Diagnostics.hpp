#include <cstdint>
#include <iostream>

#include "EngineClassData.hpp"

#ifdef Blokk_Diagnostics

namespace Blokk {

class Diagnostics 
{

private:

    EngineCore* Engine;

public:
    Diagnostics(EngineCore* TargetEngine) {
        Engine = TargetEngine;
    }

    uint32_t GetTotalObjects() const {
        return Engine->ObjectCount;
    }

    uint32_t GetStaticObjectCount() const {
        return Engine->StaticObjectCount;
    }

    uint32_t GetDynamicObjectCount() const {
        return Engine->DynamicObjectCount;
    }

    uint32_t GetOpenedThreads() const {
        return Engine->OpenedThreads;
    }

    uint32_t GetTotalThreads() const {
        return Engine->ThreadCount;
    }

    double GetFrameExecutionTime() const {
        return Engine->FrameExecutionTime;
    }

    double GetTargetExecutionTime() const {
        return Engine->TargetExecutionTime;
    }

    uint32_t GetFPS() const {
        return Engine->FPS;
    }

    double GetUserUpdateTime() const {
        return Engine->UserUpdateTime;
    }

    double GetVelocityTime() const {
        return Engine->VelocityTime;
    }

    double GetAnimationIncrementTime() const {
        return Engine->AnimationIncrementTime;
    }

    double GetVisibilityCullingTime() const {
        return Engine->VisibilityCullingTime;
    }

    #ifdef Blokk_Rendering_Enabled
    double GetRenderTime() const {
        return RenderTime;
    }
    #endif

    uint32_t GetSIMDRegisterSize() const
    {
        switch (SIMDRegisterLevel)
        {
            case SIMDLevel::AVX2:
                return 256;

            case SIMDLevel::AVX512:
                return 512;

            case SIMDLevel::NEON:
            case SIMDLevel::SSE2:
                return 128;

            default:
                return 0;
        }
    }

    std::string GetSIMDRegisterType() const
    {
        switch (SIMDRegisterLevel)
        {
            case SIMDLevel::AVX2:
                return "avx2";

            case SIMDLevel::AVX512:
                return "avx512";

            case SIMDLevel::SSE2:
                return "sse2";

            case SIMDLevel::NEON:
                return "neon";

            default:
                return "unknown";
        }
    }

    void PrintDiagnostics()
    {
        std::cout << "BLOKK Diagnostics report ==============" << "\n\n";

        std::cout << "Frames ---------------" << '\n';
        std::cout << "FPS: " << FPS << '\n';
        std::cout << "Total frame time: " << FrameTime << '\n';
        std::cout << "FrameExecutionTime: " << FrameExecutionTime << '\n';
        std::cout << "TargetExecutionTime: " << TargetExecutionTime << "\n\n";

        std::cout << "Threads ---------------" << '\n';
        std::cout << "Opened Threads: " << OpenedThreads << '\n';
        std::cout << "Total Threads: " << ThreadCount << '\n';
        std::cout << "Optimal thread count reached: "
                    << (AdaptiveThreadingEnabled ? "true" : "false")
                    << "\n\n";

        std::cout << "Object Counts ---------------" << '\n';
        std::cout << "Total object count: " << ObjectCount << '\n';
        std::cout << "Dynamic object count: " << DynamicObjectCount << '\n';
        std::cout << "Static object count: " << StaticObjectCount << "\n\n";

        std::cout << "SIMD Register ---------------" << '\n';
        std::cout << "SIMD register type: "
                    << GetSIMDRegisterType() << '\n';
        std::cout << "SIMD register size: "
                    << GetSIMDRegisterSize() << " bits" << '\n';

        std::cout << "System timing ---------------" << '\n';
        std::cout << "User update processing: "
                    << UserUpdateTime << '\n';
        std::cout << "Updating positions with velocities: "
                    << VelocityTime << '\n';
        std::cout << "Animation frame# incrementing: "
                    << AnimationIncrementTime << '\n';
        std::cout << "Visibility culling: "
                    << VisibilityCullingTime << '\n';

        #ifdef Blokk_Rendering_Enabled
        std::cout << "Rendering: "
                    << RenderTime << '\n';
        #endif

        std::cout << "END ==================================" << '\n';
    }
    };

}

#endif