#include "EngineClassData.hpp"
#include "Unordered_Queue.hpp"

#include <algorithm>
#include <chrono>
#include <vector>
#include <stdexcept>


// TODO: Fix certain brace syntax, verify project works by giving 
// AI EngineClassData.hpp and other related files.

namespace Blokk
{

#ifdef Blokk_Threading_Manual

    void EngineCore::SetThreadCount(uint32_t Count) {
        if (Count < 1 || Count > ThreadCount) {
            throw std::out_of_range(
                "BlokkForge: Invalid count passed in EngineCore::SetThreadCount."
            );
        }

        if (Count < OpenedThreads) {
            while (OpenedThreads != Count) {
                DestroyThread();
            }
        }
        else if (Count > OpenedThreads) {
            while (OpenedThreads != Count) {
                OpenThread();
            }
        }
    }

    void EngineCore::OpenThread()
    {
        if (OpenedThreads == ThreadCount)
        {
            throw std::out_of_range(
                "Blokk error: Cannot create another thread because the maximum thread count has been reached."
            );
        }

        Workers.emplace_back(std::make_unique<Worker>());
        Workers.back()->Awake();

        OpenedThreads++;
    }

    void EngineCore::DestroyThread()
    {
        if (OpenedThreads == 1)
        {
            throw std::out_of_range(
                "Blokk error: Cannot destroy another thread because the minimum thread count has been reached (1)."
            );
        }

        Workers.back()->Stop();
        Workers.pop_back();

        OpenedThreads--;
    }

#endif

// Executes all the engine processes, returning the time it took to execute all of them
double EngineCore::TimeEngineProcesses()
{
    // Get the delta time
    deltaTime = SDL_GetTicks();

    // Get the total start time
    auto TotalStartTime = std::chrono::steady_clock::now();

    // EngineProcesses-----------------------------------------

    { // Velocities

        #ifdef Blokk_Diagnostics
            auto StartTime = std::chrono::steady_clock::now();
        #endif

        // Get ranges
        std::vector<IndexRange> VelRanges =
            GetRanges(DynamicObjectCount, UsedThreads);

        // Set function
        Worker::CurrentJob = UpdateRangeOfPositions;

        // Loop through
        uint32_t Count = VelRanges.size();
        for (uint32_t i = 0; i < Count; i++)
        {
            Workers[i]->SetRange(VelRanges[i]);
        }

        #ifdef Blokk_Rendering_Enabled

        { // Animation Frame Incrementing 
            // (Done while velocities are being done by other threads)

            #ifdef Blokk_Diagnostics
                auto StartTime = std::chrono::steady_clock::now();
            #endif

            // Let the main thread increment
            IncrementFrames(FrameNums);

            #ifdef Blokk_Diagnostics
                auto EndTime = std::chrono::steady_clock::now();

                AnimationIncrementTime =
                    std::chrono::duration<double, std::milli>(
                        EndTime - StartTime
                    ).count();
            #endif
        }

        #endif

        // Wait for each to finish
        for (uint32_t i = 0; i < Count; i++)
        {
            Workers[i]->WaitUntilFinished();
        }

        #ifdef Blokk_Diagnostics
            auto EndTime = std::chrono::steady_clock::now();

            VelocityTime =
                std::chrono::duration<double, std::milli>(
                    EndTime - StartTime
                ).count();
        #endif
    }

#ifdef Blokk_Rendering_Enabled

    { // Animations

        { // Render

            #ifdef Blokk_Diagnostics
                auto StartTime = std::chrono::steady_clock::now();
            #endif

            // Get ranges
            std::vector<IndexRange> VisRanges =
                GetRanges(ObjectCount, UsedThreads);

            // Set function
            Worker::CurrentJob = CheckVisibleRange;

            // Loop
            uint32_t Count = VisRanges.size();
            for (uint32_t i = 0; i < Count; i++)
            {
                Workers[i]->SetRange(VisRanges[i]);
            }

            // Wait for each to finish
            for (uint32_t i = 0; i < Count; i++)
            {
                Workers[i]->WaitUntilFinished();
            }

            // Clear idxs
            RenderObjectIdxs.clear();

            // Loop through workers
            for (uint32_t i = 0; i < Count; i++)
            {
                RenderObjectIdxs.insert(
                    RenderObjectIdxs.end(),
                    Workers[i]->IdxResult.begin(),
                    Workers[i]->IdxResult.end()
                );
            }

            #ifdef Blokk_Diagnostics
                auto EndTime = std::chrono::steady_clock::now();

                VisibilityCullingTime =
                    std::chrono::duration<double, std::milli>(
                        EndTime - StartTime
                    ).count();
            #endif
        }
    }

#endif

#ifdef Blokk_GameObjects_UpdateFunctions
    UpdateObjects();
#endif

    // Get the end time
    auto TotalEndTime = std::chrono::steady_clock::now();

    // Get the total time
    auto TotalTime =
        std::chrono::duration<double, std::milli>(
            TotalEndTime - TotalStartTime
        ).count();

    return TotalTime;
}

#ifdef Blokk_Threading_AdaptiveTiming

// Main processing function for the user to call
// Handles thread adpating
void EngineCore::EngineProcess()
{
    FrameExecutionTime = TimeEngineProcesses();

    // If not enabled, don't try to adapt!
    if (!AdaptiveThreadingEnabled) {
        return;
    }

    // If a thread was opened previous frame
    if (ThreadOpenedPrevFrame)
    {
        // If took longer than previous frame
        if (FrameExecutionTime > PrevFrameTime)
        {
            // Stop using a thread if can
            if(UsedThreads != 1) {
                UsedThreads--;
            }
                
            // Update vars
            ThreadOpenedPrevFrame = false;
        }

        // If took too long
        else if (FrameExecutionTime > TargetExecutionTime)
        {
            // Use another thread if can
            if(UsedThreads != OpenedThreads) 
            {
                UsedThreads++;

                // Update var
                ThreadOpenedPrevFrame = true;
            } 
                
            // Do nothing if else
        }

        // If took a good time
        else 
        {
            ThreadOpenedPrevFrame = false;
        }
    }
    else // If a thread wasn't opened the previous frame
    {
        // If it took too long
        if (FrameExecutionTime > TargetExecutionTime)
        {
            // Use another thread if can
            if(UsedThreads != OpenedThreads) 
            {
                UsedThreads++;

                // Update var
                ThreadOpenedPrevFrame = true;
            }
        }
    }

    PrevFrameTime = FrameExecutionTime;
}


#elif defined(Blokk_Threading_FixedCount)

// Main function
void EngineCore::EngineProcess()
{
    TimeEngineProcesses();
}

void EngineCore::SetThreadCount(uint32_t Count)
{
    // Safety
    if(Count == 0)
    {
        throw std::invalid_argument(
            "Blokk error: Thread count must be at least 1."
        );
    }
    else if (Count > ThreadCount) // ThreadCount represents max capacity/hardware limits
    {
        throw std::invalid_argument(
            "Blokk error: Requested thread count exceeds hardware thread count."
        );
    }

    // Slide active boundaries smoothly instead of allocating/deallocating OS memory
    UsedThreads = Count;
    OpenedThreads = Count; 
}

#endif

#ifdef Blokk_GameObjects_UpdateFunctions

void EngineCore::UpdateObjects()
{
    // Set variables --------------------------

    ThreadRanges.clear();
    FullRanges.clear();

    ProcessingWorkerCount.store(0); // Workers increment variable by themselves, so reset
    RangesCompleteCount.store(0); // No ranges have been completed

    // Reset 
    AllWorkComplete.store(false);
    WorkCompleteBeingChecked.store(false);

    // Get ranges -------------------------------------------------

    // Get the ranges each thread needs to complete
    FullRanges = GetRanges(ObjectCount, UsedThreads);
    
    // Split the ranges into smaller ones
    ThreadRanges.resize(ThreadCount);
    
    // Nothing to do
    if (ObjectCount == 0) return;
    
    // Number of ranges per thread
    uint32_t RangeSize = FullRanges[0].GetSize();
    
    // Split each full range into smaller ranges and assign them
    for (uint32_t i = 0, count = FullRanges.size(); i < count; i++)
    {
        // Get ranges
        auto& Values = GetRanges(FullRanges[i].GetSize(), RangeSize);

        // Reserve
        ThreadRanges[i].reserve(
            static_cast<uint32_t>(Values.size())
        );
    
        // Add to thread's queue
        for (const auto& val : Values) {
            ThreadRanges[i].push(val);
        }
    }

    // ------------------------------------------------------------

    // Set function
    Worker::CurrentJob = UpdateRangeOfObjects;

    // Loop through ranges
    for(uint32_t i = 0, Count = FullRanges.size(); i < Count; i++)
    {
        // Assign a range (just to make the worker start working, as it
        // doesn't use the given range for updating objects)
        Workers[i]->SetRange({0, 0});
    }
}

// Each thread runs this function
void EngineCore::UpdateRangeOfObjects(Worker& worker)
{
    uint32_t idx = worker.getID();

	auto& ranges = ThreadRanges[idx];
    uint32_t remainingInQueue = ranges.size();

    // Loop over the queue until no ranges remain
    while (true)
    {
        IndexRange range;

        // Try to get range
        if (!ranges.try_pop_front(range))
            break; // Stop if fails

        // Process if succeeeds
        ProcessRange(range);
    }

    // Work for itself complete
    RangesCompleteCount.fetch_add(1);

    // Stealing 
    StealUpdateObjects(idx);
}

// Steals objects from other threads to execute
void EngineCore::StealUpdateObjects(uint32_t idx) 
{
    // Keep attempting to steal until complete
    while (!AllWorkComplete.load())
    {
        // Loop through every worker
        for (uint32_t i = 0; i < UsedThreads; i++)
        {
            if (i == idx)
                continue; // Don't steal from yourself

            IndexRange range;

            // Try to get a value
            if (ThreadRanges[i].try_pop_back(range)) {
                ProcessRange(range); // Process if succeeds
            }
        }

        // Make sure to update variables
        UpdateCompletionVariables();
    }
}


// Checks if every worker has finished processing and stores the result, thread-safe
void EngineCore::UpdateCompletionVariables() 
{
    // If the work isn't being checked
    bool expected = false;
    if (WorkCompleteBeingChecked.compare_exchange_strong(expected, true))
    {
        // Store the result
        bool allWorkersFinished = 
            ProcessingWorkerCount == 0 // No workers are processing
            && RangesCompleteCount == UsedThreads; // All ranges are processed

        AllWorkComplete.store(allWorkersFinished);
        WorkCompleteBeingChecked.store(false); // Reset
    }
}

// Processes a specific range of object update functions
void EngineCore::ProcessRange(IndexRange range)
{
    // Started working
    ProcessingWorkerCount.fetch_add(1);

	uint32_t remainingInRange = range.GetSize();
    uint32_t currentObjIdx = range.Start;

    // Keep processing until no more objects are left in the range
    while (remainingInRange != 0)
    {
        // Process the object
        ObjectInstances[currentObjIdx]->_Update();

        remainingInRange--;
        currentObjIdx++;
    }

    // Stopped working
    ProcessingWorkerCount.fetch_add(-1);
}

void EngineCore::TimeUpdateRangeOfObjects(Worker& worker)
{
    IndexRange Range = worker.getRange();

	for (uint32_t i = Range.Start, count = Range.End; i < count; i++)
	{
		// Start time
		auto start = std::chrono::steady_clock::now();

		// Run function
		ObjectInstances[i]->_Update();

		// End time
		auto end = std::chrono::steady_clock::now();

		// Store time
        auto totalTime = std::chrono::duration_cast<nanoseconds>(end - start).count();
        TotalUpdateTimes[i] += totalTime;
        TotalUpdateTime += totalTime;
	}
}

// Gets all the average update times of gameObject Update() functions
void EngineCore::GetAverageUpdateTimes() 
{
    uint32_t count = AverageUpdateTimes.size();
    for (uint32_t i = 0; i < count; i++) {
        AverageUpdateTimes[i] = TotalUpdateTimes[i] / NumberOfTrials;
    }
}

// Resets update times
void EngineCore::ResetUpdateTimes() {
	std::fill(TotalUpdateTimes.begin(), TotalUpdateTimes.end(), 0);
    std::fill(AverageUpdateTimes.begin(), AverageUpdateTimes.end(), 0);
}

#endif // Blokk_GameObjects_UpdateFunctions

// TODO: Make loading function smoother, currently can stutter
// Loads the engine. It adapts to the load, allowing other
void EngineCore::LoadEngine(uint32_t Threading_NumberOfTests)
{
    NumberOfTrials++;
    nanoseconds totalTime{ 0 };

// Load all the animations ------------------------
#ifdef Blokk_Rendering_Enabled

    if (CurrentLoadingStage == LoadingStage::LoadingAnimations) 
    {
        // Loop through animations
	    for (uint32_t i = currentProcessingIdx, count = AnimPaths.size(); i < count; i++)
	    {
            // Update processing index
            currentProcessingIdx = i + 1;

            // Start time
            auto start = std::chrono::steady_clock::now();

            // Load animation
            LoadAnimation(i);

            // End time
            auto end = std::chrono::steady_clock::now();

            // Total time
            auto time = std::chrono::duration_cast<nanoseconds>(end - start).count();
            totalTime += time;

            // If taken up the time for the frame
            if (totalTime >= FrameTime) {
                return; // Leave
            }

	    }

        // Change loading stage
        CurrentLoadingStage = LoadingStage::TimingUpdateFunctions;
    }


#endif // Blokk_Rendering_Enabled

#ifdef Blokk_GameObjects_UpdateFunctions
    
    // Time all the gameObjects update functions ------
    if (CurrentLoadingStage == LoadingStage::TimingUpdateFunctions) 
    {
        ResetUpdateTimes(); // Set all to 0
    
	    Worker::CurrentJob = TimeUpdateRangeOfObjects;
	    std::vector<IndexRange> Ranges = GetRanges(ObjectCount, UsedThreads);

        // Set each worker's assigned ranges
        uint32_t count = Ranges.size();
	    for (uint32_t i = 0; i < count; i++) {
		    Workers[i]->SetRange(Ranges[i]);
	    }

        // Wait for every worker to complete their work
        for (uint32_t i = 0; i < count; i++) {
            Workers[i]->WaitUntilFinished();
        }

        // Calculate the average times
        GetAverageUpdateTimes();

        // Change stage
        CurrentLoadingStage = LoadingStage::FilterUpdateFunctions;

        // Return if time exceeded
        if (totalTime > FrameTime.nano) {
            return;
        }
    }

    if (CurrentLoadingStage == LoadingStage::FilterUpdateFunctions)
    {
        if (TotalUpdateTime > FrameTime.nano) 
        {
            // Get the amount of time needed to cut off
            auto timeExceeded = FrameTime.nano - TotalUpdateTime;
            auto timeExceededRemaining = timeExceeded;


            // Remove the unneeded functions
            bool complete = false;
            while (!complete)
            {

                // Lowest and highest priority function levels
                uint8_t LowestPriorityLevel = UINT8_MAX;
                uint32_t HighestPriorityLevel = 0;
                for (auto& level : UpdatePriorities) {
                    if (level < LowestPriorityLevel)
                        LowestPriorityLevel = level; // lowest priority
                    if (level > HighestPriorityLevel)
                        HighestPriorityLevel = level; // highest priority 
                }

                // Do not discard functions of the highest priority level
                if (LowestPriorityLevel == HighestPriorityLevel) {
                    CurrentLoadingStage = LoadingStage::AdaptiveThreading;
                    complete = true;
                    break;
                }

                // Get the functions that are the lowest priority level
                uint32_t count = UpdatePriorities.size();
                std::vector<uint32_t> LowestPriorityFunctions;
                for (uint32_t i = 0; i < count; i++) {
                    if (UpdatePriorities[i] == LowestPriorityLevel)
                        LowestPriorityFunctions.push_back(i);
                }

                for (auto& idx : LowestPriorityFunctions) 
                {
                    // Update the time remainging to be exceeded
                    timeExceededRemaining -= AverageUpdateTimes[idx];
                    Remove_Function(idx); // Remove the function

                    // If the time exceeded has been fixed by removing the functions
                    if (timeExceededRemaining.count() <= 0) 
                    {
                        // Leave the loop
                        CurrentLoadingStage = LoadingStage::AdaptiveThreading;
                        complete = true; 
                        break;
                    }
                }
            }
        }
    }
    
    // Allow user to update their loading screen
    return;

#endif // Blokk_GameObject_UpdateFunctions

    // Adapt threading --------------------------------

#ifdef Blokk_Threading_AdaptiveTiming

    if (CurrentLoadingStage == LoadingStage::AdaptiveThreading) 
    {
        for (uint32_t i = 0; i < Threading_NumberOfTests; i++) {
            EngineProcess(); // Allow the engine to process to adapt
        }

        // Completed!
        CurrentLoadingStage = LoadingStage::Complete;
        return;
    }

#endif // Blokk_Threading_AdaptiveTiming

}

#ifdef Blokk_GameObjects_UpdateFunctions

inline void EngineCore::Remove_Function(uint32_t idx) {
    UpdateFunctionsSet[idx] = false;
    UpdatePriorities[idx] = UINT8_MAX;
    AverageUpdateTimes[idx] = nanoseconds_zero;
}

#endif

#ifdef Blokk_Rendering_Enabled

// Load all the frames of an animation using it's path
void EngineCore::LoadAnimation(uint32_t idx) 
{
    uint32_t FrameCount = AnimPaths[idx].size();

    // Resize the frames
    Frames[idx].resize(FrameCount);
    FrameWidths[idx].resize(FrameCount);
    FrameHeights[idx].resize(FrameCount);

    // Loop through each frame
    for (uint32_t j = 0; j < FrameCount; j++)
    {
        // Load the texture
        Frames[idx][j] = Renderer::LoadTexture(AnimPaths[idx][j]);

        // Get the width and height
        SDL_QueryTexture(
            Frames[idx][j],
            nullptr,
            nullptr,
            &FrameWidths[idx][j],
            &FrameHeights[idx][j]
        );
    }
}

#endif // Blokk_Rendering_Enabled


// #endif // ifdef Blokk_GameObjects_UpdateFunctions

} // namespace Blokk