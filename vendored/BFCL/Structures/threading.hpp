#include <vector>

#include "worker_thread.hpp"

struct MultithreadingSynchronizer 
{
	std::vector<std::unique_ptr<Worker>> workers;
	uint32_t threadCount; // Amount of threads opened
	uint32_t maxThreads; // Amount of threads available

	// Constructor to initialize the synchronizer with a specified number of threads
	MultithreadingSynchronizer(uint32_t threadCount) :
		threadCount(threadCount), 
		maxThreads(std::thread::hardware_concurrency())
	{
		for (uint32_t i = 0; i < threadCount; ++i)
			workers.emplace_back(std::make_unique<Worker>());
	}

	~MultithreadingSynchronizer()
	{
		// Stop all workers
		for (auto& worker : workers)
			worker->stop();
	}

	void setThreadCount(uint32_t count)
	{
		uint32_t newCount =
			(count > maxThreads) ? maxThreads : count;

		if (newCount == threadCount)
			return;

		if (newCount > threadCount)
		{
			for (uint32_t i = threadCount; i < newCount; ++i)
				workers.emplace_back(std::make_unique<Worker>());
		}
		else
		{
			for (uint32_t i = newCount; i < threadCount; ++i)
				workers[i]->stop();

			workers.resize(newCount);
		}

		threadCount = newCount;
	}

	void setTaskForAll(std::function<void()> task)
	{
		for (auto& worker : workers)
			worker->setTask(task);
	}

	// Split a number into x ranges
	[[nodiscard]] std::vector<IndexRange> GetRanges(uint32_t Length, uint32_t Count)
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

			Result.push_back(IndexRange{ CurrentStart, CurrentEnd });

			CurrentStart = CurrentEnd; // Roll forward to the next index slot
		}

		return Result;
	}
};