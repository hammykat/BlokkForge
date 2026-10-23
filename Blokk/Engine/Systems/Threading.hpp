#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <cstdint>

#include "types.hpp"

namespace Blokk
{

class EngineCore;
class Worker;

using WorkerJobFunction =
    void (EngineCore::*)(Worker&);


// Thread
class Worker
{
public:

    ~Worker() {
        Stop();
    }

    void Run()
    {
        while (Running)
        {
            std::unique_lock<std::mutex> Lock(Mutex);

            CV.wait(Lock, [this]()
            {
                return HasWork || !Running;
            });

            if (!Running)
                break;

            IndexRange RangeToProcess = TargetRange;
            HasWork = false;

            Lock.unlock();

            // Same function + same manager for everyone
            (Manager->*CurrentJob)(*this);

            // Tell manager we're finished
            {
                std::lock_guard<std::mutex> Lock(Mutex);
                Finished = true;
            }

            FinishedCV.notify_one();
        }
    }

    void SetRange(IndexRange Range)
    {
        std::lock_guard<std::mutex> Lock(Mutex);

        TargetRange = Range;
        HasWork = true;
        Finished = false;

        CV.notify_one();
    }

    bool IsFinished()
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return Finished;
    }

    void Wake() {
        CV.notify_one();
    }

    void Awake() {
        Thread = std::thread(&Worker::Run, this);
    }

    void Stop()
    {
        Running = false;
        CV.notify_one();

        if (Thread.joinable())
            Thread.join();
    }

    [[nodiscard]] inline 
    uint32_t getID() const {
        return ID;
    }

    void setID(uint32_t id) {
        ID = id;
    }

    [[nodiscard]] inline
    IndexRange getRange() const {
        return TargetRange;
    }

    void WaitUntilFinished()
    {
        std::unique_lock<std::mutex> Lock(Mutex);

        FinishedCV.wait(Lock, [this]()
        {
            return Finished;
        });
    }

    inline static EngineCore* Manager = nullptr;
    inline static WorkerJobFunction CurrentJob = nullptr;

    std::atomic<bool> Running = true;

    std::vector<uint32_t> IdxResult;

private:

    IndexRange TargetRange;
    uint32_t ID;

    std::thread Thread;

    bool HasWork = false;
    bool Finished = true;

    std::condition_variable FinishedCV;

    std::mutex Mutex;
    std::condition_variable CV;

};

}