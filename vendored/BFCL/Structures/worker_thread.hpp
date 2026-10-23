#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

class Worker
{
private:
    std::function<void()> task;

    bool running = false;
    bool hasTask = false;
    bool executing = false;

    std::mutex mutex;

    std::condition_variable hasWork;
    std::condition_variable hasFinished;

    std::thread thread;

    void run()
    {
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(mutex);

                hasWork.wait(lock, [this]
                    {
                        return hasTask || !running;
                    });

                if (!running)
                    return;

                hasTask = false;
                executing = true;
            }

            // The task runs synchronously.
            if (task)
                task();

            {
                std::lock_guard<std::mutex> lock(mutex);
                executing = false;
            }

            hasFinished.notify_all();
        }
    }

public:
    Worker()
        : running(true),
        thread(&Worker::run, this)
    {
    }

    ~Worker()
    {
        stop();
    }

    void setTask(std::function<void()> newTask)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);

            task = std::move(newTask);
            hasTask = true;
        }

        hasWork.notify_one();
    }

    void waitUntilFinished()
    {
        std::unique_lock<std::mutex> lock(mutex);

        hasFinished.wait(lock, [this]
            {
                return !hasTask && !executing;
            });
    }

    bool IsFinished()
    {
        std::lock_guard<std::mutex> lock(mutex);

        return !hasTask && !executing;
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            running = false;
        }

        hasWork.notify_one();

        if (thread.joinable())
            thread.join();
    }
};