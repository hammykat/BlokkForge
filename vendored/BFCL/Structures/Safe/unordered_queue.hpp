#include <vector>
#include <cstdint>
#include <utility>
#include <source_location>
#include <string>
#include <stdexcept>
#include <memory>

// The unordered_queue class is a data structure that allows for efficient insertion 
// and removal of items in an unordered manner. It maintains a vector of pointers to 
// items and a corresponding vector to track ownership of those items. The class 
// provides methods for adding, removing, and accessing items, while ensuring proper 
// memory management for owned items.

template <typename Type>
class unordered_queue
{
private:
	std::vector<Type*> Items; // Pointers to the items in the queue
	std::vector<uint8_t> owned; // Represents whether the queue owns the item (1) or not (0) -- Used for cleanup (freeing values it owns)
    uint32_t ItemCount = 0; // Represents the number of items in the queue

public:
    // Default constructor
    unordered_queue() = default;

    // No copying
    unordered_queue(const unordered_queue&) = delete;
    unordered_queue& operator=(const unordered_queue&) = delete;

    ~unordered_queue()
    {
        // Remove all the items
        remove_all();
    }

    // Get a reference to a value
    Type& operator[](
        uint32_t Idx,
        std::source_location Location = std::source_location::current()
        )
    {
        // Invalid index; exceeds the last index
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
        return Items;
    }

    // Get and remove the first item
    [[nodiscard]] inline
        Type pop_front(
            std::source_location Location = std::source_location::current()
        )
    {
        if (ItemCount == 0)
        {
            ThrowError(
                "Attempted to pop_front in unordered_queue, but item count is 0",
                Location
            );
        }

        Type Result =
            (owned[0] ? std::move(*Items[0]) : *Items[0]);

        remove_at(0);

        return Result;
    }

    // Get and remove the last item
    [[nodiscard]] inline
        Type pop_back(
            std::source_location Location = std::source_location::current()
        )
    {
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

        remove_at(ItemCount - 1);

        return Result;
    }

    // Get and remove an item at a specific index
    [[nodiscard]] inline
        Type pop_at(
            uint32_t idx,
            std::source_location Location = std::source_location::current()
        )
    {
        if (idx >= ItemCount)
        {
            ThrowError(
                "Index is greater than item count in unordered_queue::pop_at",
                Location
            );
        }

        Type Result =
            (owned[idx] ? std::move(*Items[idx]) : *Items[idx]);

        remove_at(idx);

        return Result;
    }

    // Add a borrowed value to the back
    inline void push_ref(Type& val)
    {
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
        // unique_ptr temporarily owns the object
        auto ptr = std::make_unique<Type>(std::move(val));

        // Add the raw pointer to Items
        Items.push_back(ptr.get());

        try
        {
            // Mark this item as owned
            owned.push_back(true);
        }
        catch (...)
        {
            // Undo the Items insertion
            Items.pop_back();

            // ptr still owns the object
            throw;
        }

        // Everything succeeded, so transfer ownership
        ptr.release();

        ++ItemCount;
    }

    // Check if there are no items
    [[nodiscard]] inline
        bool empty() const
    {
        return ItemCount == 0;
    }

    // Get the item at the front
    [[nodiscard]] inline
        Type& front(
            std::source_location Location = std::source_location::current()
        ) const
    {
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
        return ItemCount;
    }

    // Replace the items with another vector
    inline void assign(std::vector<Type*>& Target)
    {
        // Make sure the target isn't itself
        if (&Target == &Items)
            return;

        // Clear all current items
        remove_all();

        // Copy the pointers
        Items = Target;
        ItemCount = Target.size();

        // All assigned pointers are borrowed
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
        Items.reserve(Count);
        owned.reserve(Count);
    }

    // Remove an item at a specific index
    inline void remove_at(uint32_t idx)
    {
        if (idx >= ItemCount)
        {
            ThrowError(
                "Invalid index in unordered_queue::remove_at",
                std::source_location::current()
            );
        }

        ItemCount--;

        // Swap the item with the last item
        std::swap(Items[idx], Items[ItemCount]);
        std::swap(owned[idx], owned[ItemCount]);

        // Delete the removed item if we own it
        if (owned[ItemCount])
            delete Items[ItemCount];

        Items.pop_back();
        owned.pop_back();
    }

    // Delete all the objects
    inline void remove_all()
    {
        while (ItemCount > 0)
            remove_at(ItemCount - 1);
    }

    [[noreturn]] inline
        void ThrowError(
            const std::string& Message,
            const std::source_location& Location
        )
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