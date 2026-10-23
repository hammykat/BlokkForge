#include <vector>
#include <cstdint>
#include <utility>
#include <source_location>
#include <string>
#include <stdexcept>
#include <memory>
#include <mutex>

// Check unoredered_queue.hpp for more information about this class. This is a 
// concurrent version of the unordered_queue class, which allows for thread-safe 
// operations on the queue. It uses mutexes to ensure that multiple threads can 
// safely access and modify the queue without causing data races or inconsistencies.

template <typename Type>
class concurrent_unordered_queue
{
private:
    std::vector<Type*> Items; // Pointers to the items in the queue
    std::vector<uint8_t> owned; // Represents whether the queue owns the item (1) or not (0) -- Used for cleanup (freeing values it owns)
    uint32_t ItemCount = 0; // Represents the number of items in the queue

    // Lock for thread-safe operations
    mutable std::mutex Lock;

public:

    concurrent_unordered_queue() = default;

    concurrent_unordered_queue(const concurrent_unordered_queue&) = delete;
    concurrent_unordered_queue& operator=(const concurrent_unordered_queue&) = delete;

    ~concurrent_unordered_queue()
    {
        std::unique_lock<std::mutex> mtx(Lock);

        remove_all_unlocked();
    }

    // Get a reference to a value
    Type& operator[](
        uint32_t Idx,
        std::source_location Location = std::source_location::current()
        )
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (Idx >= ItemCount)
        {
            ThrowError(
                "Index is greater than item count in unordered_queue",
                Location
            );
        }

        return *Items[Idx];
    }

    // Get a pointer to a value
    Type* operator()(
        uint32_t Idx,
        std::source_location Location = std::source_location::current()
        )
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (Idx >= ItemCount)
        {
            ThrowError(
                "Index is greater than item count in unordered_queue",
                Location
            );
        }

        return Items[Idx];
    }

    // Get a reference to the internal item pointer array
    [[nodiscard]] inline
    const std::vector<Type*>& data() const
    {
        std::unique_lock<std::mutex> mtx(Lock);

        return Items;
    }

    // Get and remove the first item
    [[nodiscard]] inline
        Type pop_front(
            std::source_location Location = std::source_location::current()
        )
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (ItemCount == 0)
        {
            ThrowError(
                "Attempted to pop_front in unordered_queue, but item count is 0",
                Location
            );
        }

        Type Result =
            (owned[0] ? std::move(*Items[0]) : *Items[0]);

        remove_at_unlocked(0);

        return Result;
    }

    // Get and remove the last item
    [[nodiscard]] inline
        Type pop_back(
            std::source_location Location = std::source_location::current()
        )
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (ItemCount == 0)
        {
            ThrowError(
                "Attempted to pop_back in unordered_queue, but item count is 0",
                Location
            );
        }

        Type Result =
            (owned[ItemCount - 1]
                ? std::move(*Items[ItemCount - 1])
                : *Items[ItemCount - 1]);

        remove_at_unlocked(ItemCount - 1);

        return Result;
    }

    [[nodiscard]] inline
        bool try_pop_front(Type& Result)
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (ItemCount == 0)
            return false;

        Type Value =
            (owned[0] ? std::move(*Items[0]) : *Items[0]);

        remove_at_unlocked(0);

        Result = std::move(Value);

        return true;
    }

    [[nodiscard]] inline
        bool try_pop_back(Type& Result)
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (ItemCount == 0)
            return false;

        uint32_t Idx = ItemCount - 1;

        Type Value =
            (owned[Idx] ? std::move(*Items[Idx]) : *Items[Idx]);

        remove_at_unlocked(Idx);

        Result = std::move(Value);

        return true;
    }

    // Get and remove an item at a specific index
    [[nodiscard]] inline
        Type pop_at(
            uint32_t idx,
            std::source_location Location = std::source_location::current()
        )
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (idx >= ItemCount)
        {
            ThrowError(
                "Index is greater than item count in unordered_queue::pop_at",
                Location
            );
        }

        Type Result =
            (owned[idx] ? std::move(*Items[idx]) : *Items[idx]);

        remove_at_unlocked(idx);

        return Result;
    }

    // Add a borrowed value to the back
    inline void push_ref(Type& val)
    {
        std::unique_lock<std::mutex> mtx(Lock);

        Items.push_back(&val);

        try
        {
            owned.push_back(false);
        }
        catch (...)
        {
            Items.pop_back();
            throw;
        }

        ++ItemCount;
    }

    // Add an owned value to the back
    inline void push(Type val)
    {
        std::unique_lock<std::mutex> mtx(Lock);

        auto ptr = std::make_unique<Type>(std::move(val));

        Items.push_back(ptr.get());

        try
        {
            owned.push_back(true);
        }
        catch (...)
        {
            Items.pop_back();
            throw;
        }

        ptr.release();

        ++ItemCount;
    }

    // Check if there are no items
    [[nodiscard]] inline
        bool empty() const
    {
        std::unique_lock<std::mutex> mtx(Lock);

        return ItemCount == 0;
    }

    // Get the item at the front
    [[nodiscard]] inline
        Type& front(
            std::source_location Location = std::source_location::current()
        ) const
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (ItemCount > 0)
            return *Items[0];

        ThrowError(
            "Attempted to get front item but no items exist in unordered_queue::front",
            Location
        );
    }

    // Get the back item
    [[nodiscard]] inline
        Type& back(
            std::source_location Location = std::source_location::current()
        ) const
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (ItemCount > 0)
            return *Items[ItemCount - 1];

        ThrowError(
            "Attempted to get back item but no items exist in unordered_queue::back",
            Location
        );
    }

    // Get the number of items
    [[nodiscard]] inline
        uint32_t size() const
    {
        std::unique_lock<std::mutex> mtx(Lock);

        return ItemCount;
    }

    // Replace the items with another vector
    inline void assign(std::vector<Type*>& Target)
    {
        std::unique_lock<std::mutex> mtx(Lock);

        if (&Target == &Items)
            return;

        remove_all_unlocked();

        Items = Target;
        ItemCount = static_cast<uint32_t>(Target.size());

        owned.clear();
        owned.reserve(ItemCount);

        for (uint32_t i = 0; i < ItemCount; i++)
        {
            owned.push_back(false);
        }
    }

    // Reserve a specific number of items
    inline void reserve(uint32_t Count)
    {
        std::unique_lock<std::mutex> mtx(Lock);

        Items.reserve(Count);
        owned.reserve(Count);
    }

    // Remove an item at a specific index
    inline void remove_at(uint32_t idx)
    {
        std::unique_lock<std::mutex> mtx(Lock);

        remove_at_unlocked(idx);
    }

    // Delete all the objects
    inline void remove_all()
    {
        std::unique_lock<std::mutex> mtx(Lock);

        remove_all_unlocked();
    }

private:

    inline void remove_at_unlocked(uint32_t idx)
    {
        if (idx >= ItemCount)
        {
            ThrowError(
                "Invalid index in unordered_queue::remove_at",
                std::source_location::current()
            );
        }

        ItemCount--;

        std::swap(Items[idx], Items[ItemCount]);
        std::swap(owned[idx], owned[ItemCount]);

        if (owned[ItemCount])
            delete Items[ItemCount];

        Items.pop_back();
        owned.pop_back();
    }

    inline void remove_all_unlocked()
    {
        while (ItemCount > 0)
            remove_at_unlocked(ItemCount - 1);
    }

public:

    [[noreturn]] inline
        void ThrowError(
            const std::string& Message,
            const std::source_location& Location
        ) const
    {
        throw std::runtime_error(
            "BFCL: "
            + Message
            + " (Line: "
            + std::to_string(Location.line())
            + ", File: "
            + Location.file_name()
            + ")"
        );
    }
};