#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <source_location>

#include "types.hpp"


// The coordinate tree is an optimized data structure that allows for the storage and retrieval of items based on their coordinates in a 2D space,
// or "groups". Each group is identified by a unique pair of coordinates (x, y), and can contain multiple items. The structure allows for efficient addition, removal, and access of items within these groups.
// The tree has safety mechanisms in place to ensure that operations on the tree are valid, throwing errors when attempting to access non-existent groups or indices.
// It is optimized for performance, making it suitable for scenarios where fast access to spatially organized data is required, such as in game development or simulations.


namespace BFCL::safe
{

	
    // The hash function for the Vector2 struct, used to allow Vector2 to be used as a key in unordered_map. 
	// This function is optimized for performance, combining the x and y coordinates into a single uint32_t value.
	// The function only takes ~1-3 CPU cycles to compute (not including the time it would take to fetch the data), 
    // making it suitable for high-performance scenarios where many hash computations may be required.
    
    struct Vector2Hash
    {
        [[nodiscard]] inline
            uint32_t operator()(const Blokk::Vector2& Coord) const noexcept
        {
            return
                (static_cast<uint64_t>(static_cast<uint32_t>(Coord.x)) << 32) |
                static_cast<uint32_t>(Coord.y);
        }
    };


    template <typename Type>
    using Vecof_ReftoVec = std::vector<std::reference_wrapper<std::vector<Type>>>;

    template <typename Type>
    class CoordinateTree2D
    {

    private:
		std::vector<Vecof_ReftoVec<Type>> data; // Contains a vector of references to vectors of Type, where each reference corresponds to a group of items
		std::unordered_map<BFCL::Vector2, uint32_t, Vector2Hash> groupNames; // Contains a mapping from group coordinates to the index of the corresponding vector in 'data'
		std::vector<uint32_t> sizes; // Contains the number of items in each group
		std::vector<std::vector<uint32_t>> freeIdxs; // Contains a vector of vectors of free indices for each group, where each inner vector corresponds to a group and contains the indices of items that have been removed and can be reused
		std::vector<uint32_t> freeIdxSize; // Contains the number of free indices for each group, where each element corresponds to a group and contains the number of items that have been removed and can be reused

    public: // Allow full access to data

        [[nodiscard]] inline
            Type getItem(
                const Blokk::Vector2& GroupCoords,
                uint32_t Idx,

                // Error data
                std::source_location Location = std::source_location::current()
            ) {
            if (auto It = GroupNames.find(GroupCoords); It != GroupNames.end())
            {
                return Values[It->second][Idx];
            }
            else { // Throw error, doesn't exist!
                ThrowError("Group coordinates not found in CoordinateTree", Location);
            }
        }

        [[nodiscard]] inline
            Type& getRefToItem(
                const Blokk::Vector2& GroupCoords,
                uint32_t Idx,

                // Error data
                std::source_location Location = std::source_location::current()
            ) {
            if (auto It = groupNames.find(GroupCoords); It != groupNames.end())
            {
                return data[It->second][Idx];
            }
            else { // Throw error, doesn't exist!
                ThrowError("Group coordinates not found in CoordinateTree", Location);
            }
        }

        // Using [] with coordinates will return a ref to all the items
        [[nodiscard]] inline
            std::vector<Type>& operator[](Vector2 ChunkCoords)
        {
            return &Values[GroupNames[GroupCoords]];
        }

        // Using [] with index will return a ref to all the items
        [[nodiscard]] inline
            std::vector<Type>& operator[](
                uint32_t Idx,

                // Error data
                std::source_location Location = std::source_location::current()
             ) {
            // If index is out of range, throw an error
            if (Idx >= Values.size()) {
                ThrowError("Index out of range in CoordinateTree", Location);
            }

            return Values[Idx];
        }


        // Add a value to a group
        void addItem(
            const Blokk::Vector2& GroupCoords,
            Type Val
        ) {
            uint32_t Idx = GroupNames.at(GroupCoords);

            // If there are free indices, use one of them
            if (FreeIdxSize[Idx] > 0)
            {
                uint32_t FreeIdx = FreeIdxs[Idx].back(); // Retrieve free index
                FreeIdxs[Idx].pop_back(); // Remove last free index

                data[Idx][FreeIdx] = Val;
                --FreeIdxSize[Idx];
            }
            else { // No indexs available, push back to the end of the vector
                data[Idx].push_back(Val);
            }
        }


        // Add a vector of items to a group
        void addItems(
            const Blokk::Vector2& GroupCoords,
            const std::vector<Type>& Items
        ) {
			auto It = groupNames.find(GroupCoords);

			// If the group doesn't exist, throw an error
			if (It == groupNames.end()) {
				ThrowError("Group coordinates not found in CoordinateTree", std::source_location::current());
			}

			uint32_t Idx = It->second;

            for (const auto& Val : Items)
            {
                if (FreeIdxSize[Idx] > 0)
                {
                    uint32_t FreeIdx = FreeIdxs[Idx].back();
                    FreeIdxs[Idx].pop_back();

                    data[Idx][FreeIdx] = Val;
                    --FreeIdxSize[Idx];
                }
                else
                {
                    data[Idx].push_back(Val);
                }
            }
        }

        // Create a new group with empty vectors
        inline void CreateNewGroup(const Blokk::Vector2 GroupCoords)
        {
            uint32_t Idx = static_cast<uint32_t>(data.size());

            groupNames.emplace(GroupCoords, Idx);
            data.emplace_back();
            FreeIdxs.emplace_back();
            FreeIdxSize.push_back(0);
        }

        // Create a new group with initial values
        inline void CreateNewGroup(
            const Blokk::Vector2 GroupCoords,
            std::vector<Type> values
        ) {
            uint32_t Idx = static_cast<uint32_t>(values.size());

            groupNames.emplace(GroupCoords, Idx);
            data.emplace_back();
            FreeIdxs.emplace_back();
            FreeIdxSize.push_back(0);
        }

        // Remove an item from a group using group coordinates
        inline void RemoveItem(
            const Blokk::Vector2& GroupCoords,
            uint32_t Idx,

            // Error data
            std::source_location Location = std::source_location::current()
        ) {
            // Find the group index using the coordinates
            const auto It = GroupNames.find(GroupCoords);

            GroupIdx = It->second;
            FreeIdxs[GroupIdx].push_back(Idx);
            ++FreeIdxSize[GroupIdx];
        }

        // Remove an item
        inline void RemoveItemFromGroup(
            uint32_t GroupIdx,
            uint32_t Idx
        ) {
			if (GroupIdx >= data.size()) {
				ThrowError("Group index out of range in CoordinateTree", std::source_location::current());
			}

            // Add the index to the free indices vector for the group to be reused later
            FreeIdxs[GroupIdx].push_back(Idx);
            ++FreeIdxSize[GroupIdx];
        }


        [[nodiscard]] inline
            uint32_t GetIndexOfGroup(
                const Blokk::Vector2& GroupCoords,

                // Error data
                std::source_location Location = std::source_location::current()
            ) {
            // Find the group index using the coordinates
            const auto It = GroupNames.find(GroupCoords);

            // If the group exists, return its index
            if (It != GroupNames.end()) {
                return It->second;
            }

            // Else throw an error
            ThrowError("Group coordinates not found in CoordinateTree", Location);
        }


        [[noreturn]] inline void
            ThrowError(const std::string& Message, const std::source_location& Location)
        {
            throw std::runtime_error(
                "BFCL: "
                + Message +
                " (Line: " + std::to_string(Location.line()) +
                ", File: " + Location.file_name() + ")"
            );
        }
    };

}