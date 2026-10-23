
#include <algorithm>
#include <iostream>
#include <cstdint>

#include "threading.hpp"

template <typename Type, uint32_t capacity>
class unordered_table
{

private:
	Type data[capacity]; // Items
	uint32_t size; // Number of items

	MultithreadingSynchronizer* synchronizer = nullptr; // Synchronizer for multithreading

public:

	unordered_table(MultithreadingSynchronizer* Sync = nullptr) :
		size(0),
		synchronizer(Sync)
	{
	}


	// Add an item to the back
	inline void add(Type& Item)
	{
		data[size] = Item;
		size++;
	}

	// Get an item at an index
	[[nodiscard]] inline
		Type get(uint32_t Idx) {
		return data[Idx];
	}

	// Swap 2 items at indexes with each other
	inline void swap(uint32_t Idx1, uint32_t Idx2) {
		std::swap(data[Idx1], data[Idx2]);
	}

	// Remove an item from the back
	[[nodiscard]] inline
		Type pop_back()
	{
		size--;
		return data[size];
	}

	// Remove an item at an index
	[[nodiscard]] inline
		Type pop(uint32_t Idx)
	{
		// Swap with last item
		std::swap(data[Idx], data[size - 1]);
		size--;
		return data[size];
	}

	// Get the maximum capacity
	[[nodiscard]] inline
		uint32_t getMaxSize() {
		return capacity;
	}

	// Get the current number of items
	[[nodiscard]] inline
		uint32_t getSize() {
		return size;
	}

	void copy(
		bool useMultipleThreads,
		std::vector<Type>& items,

		// Error data
		std::source_location Location = std::source_location::current()
	) {
		uint32_t copySize = static_cast<uint32_t>(
			std::min<size_t>(items.size(), capacity)
			);

		if (items.size() > capacity) {
			std::cerr
				<< "BFCL Warning: unordered_table capacity exceeded. "
				<< "Only copying " << capacity
				<< " of " << items.size()
				<< " items. "
				<< "(Line: " << Location.line()
				<< ", File: " << Location.file_name()
				<< ")\n";
		}

		if (useMultipleThreads) {
			// Copy using multiple threads

			// If not set, throw an error
			if (synchronizer == nullptr) {
				ThrowError("MultithreadingSynchronizer is not set for unordered_table", Location);
			}

			// Give them each a task to copy their portion of the items
			uint32_t count = synchronizer->threadCount;
			for (uint32_t i = 0; i < count; i++) {
				synchronizer->workers[i]->setTask([this, &items, i, count, copySize] {
					for (uint32_t j = i; j < copySize; j += count) {
						data[j] = items[j];
					}
					});
			}

			// Wait until all are finished
			for (uint32_t i = 0; i < count; i++) {
				synchronizer->workers[i]->waitUntilFinished();
			}
		}
		else {
			// Copy using a single thread
			for (uint32_t i = 0; i < copySize; i++) {
				data[i] = items[i];
			}
		}

		size = copySize;
	}

	[[noreturn]] inline void ThrowError(const std::string& Message, const std::source_location& Location)
	{
		throw std::runtime_error(
			"BFCL: "
			+ Message +
			" (Line: " + std::to_string(Location.line()) +
			", File: " + Location.file_name() + ")"
		);
	}
};