#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <SDL3/SDL.h>

// BFCL
#include <concurrent_unordered_queue.hpp>

#include "Camera.hpp"

// Forward-declare SDL types to avoid pulling the whole SDL header into this
// widely-included header. Include the SDL headers in .cpp files where
// concrete SDL types are required.
struct SDL_Texture;

// Configuration macros are handled in CMake. Check "Cmake/JSONParser.cmake"

#include "types.hpp"
#include "SIMD_Finder.hpp"
#include "GameObject.hpp"


namespace Blokk
{

    class EngineCore;
    class GameObject;
    class Renderer;
    class Diagnostics;


    using UpdatePositionsFnPtr =
        void(EngineCore::*)(
            float* PosX, float* PosY,
            const float* VelX, const float* VelY,
            uint32_t Size
        );

    using TimeEngineProcessesFnPtr =
        double(EngineCore::*)();

    using nanoseconds = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;
    using seconds = std::chrono::seconds;

    constexpr auto nanoseconds_zero = std::chrono::nanoseconds::zero();
}

#include "Threading.hpp"

namespace Blokk
{
    using CheckVisibleRangeFnPtr =
        void(EngineCore::*)(Worker& Thread);

    enum Culling
    {
        Axis,
        Basic
    };

    struct EngineDiagnostics
    {
        uint32_t TotalThreadCount, OpenedThreadCount;
    };

    struct ManagerCreation
    {
        vector2<uint32_t> ScreenDimensions;
        uint32_t FPS = 30;
    };

    // Object manager
    class EngineCore
    {
    public:
        friend class GameObject;
        friend class CachedGameObject;
        friend class Renderer;
        friend class Diagnostics;

        EngineCore(ManagerCreation Cr) :

            // Threads
            ThreadOpenedPrevFrame(false),
            ThreadDestroyedPrevFrame(false),
            AdaptiveThreadingEnabled(true),
            ThreadCount(std::thread::hardware_concurrency()),
            OpenedThreads(ThreadCount),
            PrevOpenedThreads(0),

            // Frames
            FPS(Cr.FPS),
            FrameTime {
                std::chrono::duration<double>{1.0 / Cr.FPS},
                std::chrono::duration<double, std::milli>{1.0 / Cr.FPS},
                std::chrono::duration<double, std::micro>{1.0 / Cr.FPS},
                std::chrono::duration<double, std::nano>{1.0 / Cr.FPS}
            },
            TargetExecutionTime {
                FrameTime.secs * 0.8,
                FrameTime.milli * 0.8,
                FrameTime.micro * 0.8,
                FrameTime.nano * 0.8
            },

            FrameExecutionTime(0),
            PrevFrameTime(0),

            // Diagnostics timing
            #ifdef Blokk_Diagnostics
            UserUpdateTime(0),
            VelocityTime(0),
            AnimationIncrementTime(0),
            VisibilityCullingTime(0),

                #ifdef Blokk_Rendering_Enabled
                    RenderTime(0),
                #endif

            #endif

            // Camera
            #ifdef Blokk_CamEnabled
                Camera(),
            #endif

            // SIMD
            SIMDRegisterLevel(DetectSIMD()),

            // Screen
            ScreenWidth(Cr.ScreenDimensions.x),
            ScreenHeight(Cr.ScreenDimensions.y),

            // Counts
            ObjectCount(0),
            StaticObjectCount(0),
            DynamicObjectCount(0)
        {

            // Throw an error if no threads were found
            if (ThreadCount == 0)
            {
                throw std::runtime_error(
                    "Blokk couldn't find the hardware thread count."
                );
            }

            // Set the gameobject's manager
            GameObject::Engine = this;

            // Set the worker's manager
            Worker::Manager = this;

            // Open all threads
            for(uint32_t i = 0; i < ThreadCount; i++)
            {
                OpenThread();
                Workers[i]->setID(i);
            }

            // Get the proper functions
            GetFunctions();
        }

        ~EngineCore()
        {
            // Destroy all threads
            for(uint32_t i = 0; i < OpenedThreads; i++) {
                DestroyThread();
            }
        }

        void EngineProcess();

        // Camera
        #ifdef Blokk_CamEnabled
            CameraController Camera;
        #endif

        // Functions the user can call if manual threading enabled
        #ifdef Blokk_Threading_Manual
        void SetThreadCount(uint32_t Count); // Set the number of threads
        void OpenThread(); // Create a thread
        void DestroyThread(); // Destroy a thread
        #endif

        #ifdef Blokk_Threading_AdaptiveTiming

        // Adaptive threading system flags
        void StopAdaptiveThreadingSystem() { // Disable
            AdaptiveThreadingEnabled = false;
        }
        void EnableAdaptiveThreadingSystem() { // Enable
            AdaptiveThreadingEnabled = true;
        }

        void SetTargetExecutionTime(double Time)
        {
            // Make sure time isn't negative
            if (Time <= 0)
            {
                throw std::invalid_argument(
                    "Blokk error: Tried to set frame time to negative value or 0."
                );
            }

            TargetExecutionTime = Time;
        }

        #endif

    // PRIVATE -------------------------------------------
    private:

        SIMDLevel SIMDRegisterLevel;
        uint32_t ScreenHeight;
        uint32_t ScreenWidth;

        // Animations
        #ifdef Blokk_Rendering_Enabled

        std::vector<uint8_t> IsVisible;
        std::vector<uint32_t> FrameNums;
        std::vector<uint32_t> AnimNums;

        // Stores the animation names, points to an index in the animations
        std::unordered_map<std::string, uint32_t> AnimNames;

        // Stores a list of animations
		std::vector<std::vector<std::filesystem::path>> AnimPaths; // Contains the paths to the animation frames for each animation
		std::vector<std::vector<SDL_Texture*>> Frames; // Contains the loaded SDL_Texture* for each animation frame

        // Contains the frame dimensions for each animation frame
		std::vector<std::vector<float>> FrameWidths; 
        std::vector<std::vector<float>> FrameHeights;

        std::vector<uint32_t> AnimFrameCounts; // Contains the number of frames for each animation

        // Visibility
        std::vector<float> AnimHeights;
        std::vector<float> AnimWidths;

        #endif

        // Pointers to object instances
        std::vector<GameObject*> ObjectInstances;

        // Rendering
        std::vector<uint32_t> RenderObjectIdxs;

        // Frmae timings -----------------------------
        struct Timings
        {
            std::chrono::duration<double> secs{0};
            std::chrono::duration<double, std::milli> milli;
            std::chrono::duration<double, std::micro> micro;
            std::chrono::duration<double, std::nano> nano = nanoseconds_zero;
        };
        Timings FrameTime;
        Timings TargetExecutionTime;
        uint32_t FPS;
        Uint64 deltaTime;

        // TODO: Add a new vector of booleans to detect whether the function has actually been set
        // to avoid calling an empty for a potential performance increase

        // Work stealing system (for gameObject update functions)
        #ifdef Blokk_GameObjects_UpdateFunctions

        uint32_t NumberOfTrials = 0;
        uint32_t currentProcessingIdx = 0;
        std::vector<nanoseconds> TotalUpdateTimes; // Represents the total update time for each function
        std::vector<nanoseconds> AverageUpdateTimes; // Represents the average amount of time each functoin took to execute
        bool ImprovedUpdateTime; // Represents if the update time from this frame is less than the previous
        std::vector<uint8_t> UpdatePriorities; // Represents the game object update function priority levels
        std::vector<uint8_t> UpdateFunctionsSet; // Represents if the update function has been set
        nanoseconds TotalUpdateTime; // Total amount of time for all functions to update
        nanoseconds AverageUpdateTime; // Average amount of time for all the functions to update
        nanoseconds PrevAvgUpdateTime;
        bool ImprovedUpdateTimes; // If the update times from this frame were better that previous frames (shorter)

        uint8_t LowestUpdatePriorityLevel;
        uint8_t HighestUpdatePriorityLevel;
        std::vector<uint32_t> LowestUpdatePriorityFunctions;

        std::vector<concurrent_unordered_queue<IndexRange>> ThreadRanges; // Represents every thread's queue of ranges to complete
        std::vector<IndexRange> FullRanges; // Represent the full ranges assigned to each thread at the start
        std::atomic<uint32_t> ProcessingWorkerCount = 0; // Represents the number of workers currently processing
        std::atomic<uint32_t> RangesCompleteCount; // Represents the number of workers that have their ranges complete OR the last range is being currently processed
        std::atomic<bool> AllWorkComplete; // If all the ranges have been processed
        std::atomic<bool> WorkCompleteBeingChecked;

        #endif // Blokk_GameObjects_UpdateFunctions

		enum class LoadingStage
		{
			LoadingAnimations, // Currently loading all the animations
			TimingUpdateFunctions, // Currently timing all the gameObject update functions
            FilterUpdateFunctions, // Currently getting rid of unneeded update functions if taking too long
			AdaptiveThreading, // Currently adapting the threading system to the current load
			Complete // Finished loading the engine
		};

        // Allow the user to create their own custom loading screen for their game! --------------------------------------
		LoadingStage CurrentLoadingStage; // Current loading stage of the engine
		float CurrentPercentage; // Current percentage of the current loading stage
		uint32_t CurrentLoadingObjectIdx; // Current object index being processed for the current loading stage
        uint32_t NumberOfTrials; 

        // Workers
		uint32_t ThreadCount; // Total number of threads available on the system
		uint32_t OpenedThreads; // Total number of threads currently opened by the engine
		uint32_t UsedThreads; // Total number of threads currently being used by the engine
        uint32_t PrevOpenedThreads;

        bool ThreadOpenedPrevFrame;
        bool ThreadDestroyedPrevFrame;
        bool AdaptiveThreadingEnabled;

        double FrameExecutionTime;
        double PrevFrameTime;
        double TargetExecutionTime;
        

        std::vector<std::unique_ptr<Worker>> Workers;

        #ifdef Blokk_Diagnostics

        double UserUpdateTime;
        double VelocityTime;
        double AnimationIncrementTime;
        double VisibilityCullingTime;

            #ifdef Blokk_Rendering_Enabled
                double RenderTime;
            #endif

        #endif

        // Counts
        uint32_t ObjectCount;
        uint32_t StaticObjectCount;
        uint32_t DynamicObjectCount;

        // Functions -------------------------------------------------

        // Get the right function implementations according to the user's SIMD
        void GetFunctions()
        {
            #if (Blokk_Visibility_CullType == 0)
                CheckVisibleRange = CheckVisibilityFn_Basic;
            #elif (Blokk_Visibility_CullType == 1)
                CheckVisibleRange = CheckVisibilityFn_Axis;
            #endif
        }

        // Split a number into x ranges
        std::vector<IndexRange> GetRanges(uint32_t Length, uint32_t Count)
        {
            if (Count == 0 || Length == 0)
                return {};

            uint32_t NewCount = std::min(Length, Count);

            uint32_t BaseSize = Length / NewCount;
            uint32_t Remainder = Length % NewCount; // Find out how many extra items are leftover

            std::vector<IndexRange> Result;
            Result.reserve(NewCount);

            uint32_t CurrentStart = 0;

            for (uint32_t i = 0; i < NewCount; i++)
            {
                // Give 1 extra item from the remainder pool to the first few threads
                uint32_t CurrentSize = BaseSize + (i < Remainder ? 1 : 0);
                uint32_t CurrentEnd = CurrentStart + CurrentSize;

                Result.push_back(IndexRange{CurrentStart, CurrentEnd});
                
                CurrentStart = CurrentEnd; // Roll forward to the next index slot
            }

            return Result;
        }

        void GetRanges(uint32_t Length, uint32_t Count, std::vector<IndexRange>* Result) {
            *Result = GetRanges(Length, Count);
        }


        

        // Time engine processes -------------------------------------

        // Execute engine processes and time it
        double TimeEngineProcesses();

        // Swap 2 objects
        void SwapStaticObjects(uint32_t Obj1, uint32_t Obj2);
        void SwapDynamicObjects(uint32_t Obj1, uint32_t Obj2);

        // Destroy an object
        void DestroyDynamicObject(uint32_t ObjIdx);
        void DestroyStaticObject(uint32_t ObjIdx);

        // POSITIONS W/ VELS -----------------------------------------

        void UpdateRangeOfPositions(Worker& worker);

        UpdatePositionsFnPtr UpdatePositions;

        void UpdatePositionsFn(
            float* PosX,
            float* PosY,
            const float* VelX,
            const float* VelY,
            uint32_t Size
        );

        // Visibility checks ------------------------------------------

        #if (Blokk_Visibility_CullType == 0)

            void CheckVisibilityFn_Basic(Worker& worker);

        #elif (Blokk_Visibility_CullType == 1)

            void CheckVisibilityFn_Axis(Worker& worker);

        #endif

        CheckVisibleRangeFnPtr CheckVisibleRange;

        // Adding objects ----------------------------------------------------

        void ProcessAddCommand(EngineObjectCreationParams Fields);

        void ProcessAddCommand(bool Visible, vector2<float> Velocity, vector2<float> Position, GameObject* Object);

        // Frame increment --------------------------------------------

        void IncrementFrames(
            std::vector<uint32_t>& FrameNums,
            uint32_t Size
        );

        // GameObject update functions
        #ifdef Blokk_GameObject_UpdateFunctions

        void UpdateObjects();
        void TimeUpdateRangeOfObjects(Worker& worker);
        void ResetUpdateTimes();
        void UpdateTimeVariables();
        void ProcessRange(IndexRange range);
        void UpdateRangeOfObjects(Worker& worker);
        void UpdateCompletionVariables();
        void StealUpdateObjects(uint32_t idx);
        void GetAverageUpdateTimes();
        void Remove_Function(uint32_t idx);
        
        #endif

        void LoadEngine(uint32_t Threading_NumberOfTests);
    };
}