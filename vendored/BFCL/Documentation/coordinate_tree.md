# CoordinateTree2D

`CoordinateTree2D` is a data structure for organizing items into groups based on their 2D coordinates.

Each group is identified by an `(x, y)` coordinate and contains its own collection of items. This makes it useful for applications that work with spatial data, such as games, simulations, and other 2D systems.

## How It Works

A `CoordinateTree2D` is organized into three main parts:

```text
Coordinate
    ↓
  Group
    ↓
  Items
```

For example:

```text
(0, 0) → [Player, Enemy, Enemy]
(1, 0) → [Tree, Rock]
(0, 1) → [Enemy, Coin, Coin]
```

Each coordinate represents one group, and each group can contain any number of items.

Groups can be accessed either by their coordinates or by their internal index.

## Creating a CoordinateTree2D

```cpp
BFCL::safe::CoordinateTree2D<int> Tree;
```

This creates a tree that stores `int` values.

You can use any type that can be stored inside a `std::vector`.

## Creating Groups

Create an empty group by providing its coordinates:

```cpp
Tree.CreateNewGroup({0, 0});
Tree.CreateNewGroup({1, 0});
Tree.CreateNewGroup({0, 1});
```

You can then add items to each group:

```cpp
Tree.addItem({0, 0}, 10);
Tree.addItem({0, 0}, 20);

Tree.addItem({1, 0}, 30);
```

The resulting structure would look approximately like:

```text
(0, 0) → [10, 20]
(1, 0) → [30]
(0, 1) → []
```

## Adding Multiple Items

Multiple items can be added at once using `addItems()`:

```cpp
std::vector<int> Values = {10, 20, 30, 40};

Tree.addItems({0, 0}, Values);
```

The values are added to the group in the same order as the input vector.

## Accessing Items

### Accessing a Single Item

Use `getItem()` when you only need the value:

```cpp
int Value = Tree.getItem({0, 0}, 2);
```

Use `getRefToItem()` when you want to modify the original item:

```cpp
Tree.getRefToItem({0, 0}, 2) = 100;
```

The difference is that `getItem()` returns a copy, while `getRefToItem()` returns a reference to the stored item.

## Accessing a Group

A group can be accessed using its coordinates:

```cpp
std::vector<int>& Items = Tree[{0, 0}];
```

You can also access a group using its index:

```cpp
std::vector<int>& Items = Tree[0];
```

The index of a group can be found with:

```cpp
uint32_t GroupIndex = Tree.GetIndexOfGroup({0, 0});
```

This can be useful when repeatedly accessing the same group, since the coordinate lookup only needs to be performed once.

## Removing Items

Items can be removed using either their coordinates or the group's index.

Using coordinates:

```cpp
Tree.RemoveItem({0, 0}, 2);
```

Using the group index:

```cpp
Tree.RemoveItemFromGroup(0, 2);
```

### Reusing Removed Indices

When an item is removed, its index is saved instead of immediately removing the element from the underlying vector.

For example:

```text
Before:

[10, 20, 30, 40]
          ↑
        index 2
```

After removing index `2`:

```text
[10, 20, 30, 40]
          ↑
     free index
```

If another item is added later, the tree can reuse that index instead of adding another element to the end.

This helps reduce unnecessary growth of the underlying vectors when items are frequently added and removed.

## Performance

`CoordinateTree2D` is designed around simple data structures from the C++ standard library:

* `std::vector` stores group data.
* `std::unordered_map` finds groups from their coordinates.
* Removed indices are stored and reused.
* Groups can be accessed directly once their index is known.

The coordinate lookup uses a custom `Vector2Hash` so that `Vector2` can be used efficiently as a key in the `unordered_map`.

For applications that repeatedly access the same group, storing the group's index can avoid repeatedly searching for its coordinates.

## Safety

The `safe` version of `CoordinateTree2D` performs checks for invalid operations and reports errors through BFCL's error system.

For example, attempting to access a group that does not exist will produce an error containing the location where the operation was performed.

This makes the `safe` version useful when debugging or when correctness is more important than avoiding every possible safety check.

## Example

A simple example using `CoordinateTree2D`:

```cpp
#include <vector>

#include "CoordinateTree2D.hpp"

int main()
{
    BFCL::safe::CoordinateTree2D<int> Tree;

    // Create groups
    Tree.CreateNewGroup({0, 0});
    Tree.CreateNewGroup({1, 0});

    // Add items
    Tree.addItem({0, 0}, 10);
    Tree.addItem({0, 0}, 20);

    Tree.addItem({1, 0}, 30);

    // Access an item
    int Value = Tree.getItem({0, 0}, 1);

    // Modify an item
    Tree.getRefToItem({0, 0}, 1) = 100;

    // Remove an item
    Tree.RemoveItem({0, 0}, 0);
}
```

## When Should I Use CoordinateTree2D?

`CoordinateTree2D` is useful when your data naturally belongs to 2D groups.

Some examples include:

* Game world chunks
* Spatial partitions
* Tile-based worlds
* Simulation regions
* Object groups
* Grid-based systems
* 2D maps

If your data does not need to be organized by coordinates, a regular `std::vector` or another BFCL container may be simpler.

## Summary

`CoordinateTree2D` provides a simple way to organize data into coordinate-based groups:

```text
        CoordinateTree2D
               │
       ┌───────┼───────┐
       ↓       ↓       ↓
     (0,0)   (1,0)   (0,1)
       │       │       │
       ↓       ↓       ↓
   [Items]  [Items]  [Items]
```

It combines coordinate-based lookup, contiguous group storage, and reusable indices into one structure designed for spatially organized data.
