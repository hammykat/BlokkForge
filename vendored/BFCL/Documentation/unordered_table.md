# `unordered_table`

`unordered_table` is a fixed-capacity container for storing objects in a contiguous array.

It is designed for situations where you know the maximum number of elements you want to store ahead of time.

It also supports copying data using multiple persistent worker threads through `MultithreadingSynchronizer`.

---

## Include

```cpp
#include "unordered_table.hpp"
```

If you want to use multithreaded copying, you will also need your threading system:

```cpp
#include "threading.hpp"
```

---

# Creating an `unordered_table`

`unordered_table` takes two template parameters:

```cpp
unordered_table<Type, capacity>
```

* `Type` — The type of object stored in the table.
* `capacity` — The maximum number of objects the table can store.

For example:

```cpp
unordered_table<int, 100> Numbers;
```

This creates a table that can store up to `100` integers.

Another example:

```cpp
unordered_table<MyObject, 1000> Objects;
```

This can store up to `1000` `MyObject` objects.

The capacity is fixed when the table is created.

---

# Creating an `unordered_table` With Multithreading

You can optionally give the table a `MultithreadingSynchronizer`.

```cpp
MultithreadingSynchronizer Synchronizer(4);

unordered_table<int, 1000> Numbers(&Synchronizer);
```

The table can now use the synchronizer when `copy()` is called with multithreading enabled.

If you don't want multithreading:

```cpp
unordered_table<int, 1000> Numbers;
```

The synchronizer defaults to `nullptr`.

---

# `add()`

Adds an item to the end of the table.

```cpp
Numbers.add(Value);
```

Example:

```cpp
unordered_table<int, 100> Numbers;

int Number = 42;

Numbers.add(Number);
```

The table now contains:

```text
[42]
```

Another item:

```cpp
int Number2 = 17;

Numbers.add(Number2);
```

Now:

```text
[42, 17]
```

`add()` increases the table's size by one.

> Make sure the table has enough capacity before adding an item.

---

# `get()`

Gets an item at a specific index.

```cpp
Numbers.get(Index);
```

Example:

```cpp
unordered_table<int, 100> Numbers;

int A = 10;
int B = 20;

Numbers.add(A);
Numbers.add(B);

int Value = Numbers.get(1);
```

`Value` will contain:

```text
20
```

Indexes start at `0`.

```text
Index:  0   1
        ↓   ↓
Data:  [10, 20]
```

---

# `swap()`

Swaps two elements in the table.

```cpp
Numbers.swap(Index1, Index2);
```

Example:

```cpp
unordered_table<int, 100> Numbers;

int A = 10;
int B = 20;
int C = 30;

Numbers.add(A);
Numbers.add(B);
Numbers.add(C);
```

The table contains:

```text
[10, 20, 30]
```

Now:

```cpp
Numbers.swap(0, 2);
```

The table becomes:

```text
[30, 20, 10]
```

This is useful when you need to rearrange elements without moving the entire container.

---

# `pop_back()`

Removes the last item from the table and returns it.

```cpp
Type Item = Numbers.pop_back();
```

Example:

```cpp
unordered_table<int, 100> Numbers;

int A = 10;
int B = 20;
int C = 30;

Numbers.add(A);
Numbers.add(B);
Numbers.add(C);
```

The table:

```text
[10, 20, 30]
```

Then:

```cpp
int Removed = Numbers.pop_back();
```

The result is:

```text
Removed = 30
```

And the table becomes:

```text
[10, 20]
```

The size decreases by one.

---

# `pop()`

Removes an item at a specific index.

```cpp
Type Item = Numbers.pop(Index);
```

The item is swapped with the last item before being removed.

For example:

```cpp
unordered_table<int, 100> Numbers;

int A = 10;
int B = 20;
int C = 30;
int D = 40;

Numbers.add(A);
Numbers.add(B);
Numbers.add(C);
Numbers.add(D);
```

The table is:

```text
[10, 20, 30, 40]
```

Now:

```cpp
int Removed = Numbers.pop(1);
```

Index `1` contains `20`.

The table swaps it with the last element:

```text
[10, 40, 30, 20]
```

Then removes the last element:

```text
[10, 40, 30]
```

The returned value is:

```text
Removed = 20
```

This means `pop()` does **not preserve ordering**.

That is intentional and allows the removal to be very cheap.

---

# `getMaxSize()`

Returns the maximum capacity of the table.

```cpp
uint32_t Capacity = Numbers.getMaxSize();
```

Example:

```cpp
unordered_table<int, 1000> Numbers;

uint32_t Capacity = Numbers.getMaxSize();
```

`Capacity` will be:

```text
1000
```

The value does not change during the lifetime of the table.

---

# `getSize()`

Returns the current number of elements stored in the table.

```cpp
uint32_t Size = Numbers.getSize();
```

Example:

```cpp
unordered_table<int, 100> Numbers;

int A = 10;
int B = 20;

Numbers.add(A);
Numbers.add(B);

uint32_t Size = Numbers.getSize();
```

`Size` will be:

```text
2
```

The difference between `getSize()` and `getMaxSize()` is:

```text
getSize()     → How many items are currently stored
getMaxSize()  → How many items can be stored
```

For example:

```text
Capacity = 100

Current:
[ A B C D ... ]

getSize()    = 4
getMaxSize() = 100
```

---

# `copy()`

Copies elements from a `std::vector` into the table.

```cpp
Numbers.copy(false, Items);
```

The first parameter controls whether the copy uses multiple worker threads.

```cpp
copy(useMultipleThreads, items);
```

* `true` → use the `MultithreadingSynchronizer`
* `false` → perform the copy on the calling thread

---

## Single-threaded copy

Example:

```cpp
std::vector<int> Items =
{
    10,
    20,
    30,
    40
};

unordered_table<int, 100> Numbers;

Numbers.copy(false, Items);
```

The table becomes:

```text
[10, 20, 30, 40]
```

The table's size is automatically updated.

```cpp
Numbers.getSize(); // 4
```

---

# Multithreaded copy

To use multithreading, first create a `MultithreadingSynchronizer`.

```cpp
MultithreadingSynchronizer Synchronizer(4);
```

This creates four persistent workers.

Then give the synchronizer to the table:

```cpp
unordered_table<int, 1000> Numbers(&Synchronizer);
```

Now you can do:

```cpp
std::vector<int> Items =
{
    10,
    20,
    30,
    40,
    50,
    60,
    70,
    80
};

Numbers.copy(true, Items);
```

The copy is split between the workers.

Conceptually:

```text
Items:
[0 1 2 3 4 5 6 7]

Worker 0 → 0, 4
Worker 1 → 1, 5
Worker 2 → 2, 6
Worker 3 → 3, 7
```

The `copy()` function waits until all workers have finished before returning.

Therefore:

```cpp
Numbers.copy(true, Items);

// All copying is finished here.
```

---

# Copy Capacity

The table has a fixed maximum capacity.

For example:

```cpp
unordered_table<int, 5> Numbers;
```

can store a maximum of:

```text
5 items
```

If you copy more than five items:

```cpp
std::vector<int> Items =
{
    1, 2, 3, 4, 5, 6, 7, 8
};

Numbers.copy(false, Items);
```

the table will issue a warning.

It will copy as many items as it can:

```text
Table:

[1, 2, 3, 4, 5]
```

The extra items are not copied:

```text
6, 7, 8
```

The table's size becomes the number of items actually copied:

```cpp
Numbers.getSize(); // 5
```

This behavior is the same for single-threaded and multithreaded copying.

---

# Complete Example

Here is a small example using most of the functions:

```cpp
#include <iostream>
#include <vector>

#include "unordered_table.hpp"

int main()
{
    unordered_table<int, 100> Numbers;

    int A = 10;
    int B = 20;
    int C = 30;

    // Add items
    Numbers.add(A);
    Numbers.add(B);
    Numbers.add(C);

    // Get an item
    std::cout << Numbers.get(1) << '\n';

    // Swap items
    Numbers.swap(0, 2);

    // Remove the last item
    int Last = Numbers.pop_back();

    // Remove an item at an index
    int Removed = Numbers.pop(0);

    // Get current size
    std::cout << Numbers.getSize() << '\n';

    // Get maximum capacity
    std::cout << Numbers.getMaxSize() << '\n';
}
```

---

# Multithreading Example

```cpp
#include <iostream>
#include <vector>

#include "unordered_table.hpp"
#include "threading.hpp"

int main()
{
    // Create four persistent worker threads
    MultithreadingSynchronizer Synchronizer(4);

    // Give the table access to the synchronizer
    unordered_table<int, 1000> Numbers(&Synchronizer);

    std::vector<int> Items;

    for (int i = 0; i < 1000; i++)
    {
        Items.push_back(i);
    }

    // Copy using the worker threads
    Numbers.copy(true, Items);

    std::cout << "Copied "
              << Numbers.getSize()
              << " items.\n";
}
```

The important part is:

```cpp
MultithreadingSynchronizer Synchronizer(4);

unordered_table<int, 1000> Numbers(&Synchronizer);

Numbers.copy(true, Items);
```

The workers are created once and can be reused for future multithreaded operations.

---

# Function Summary

| Function               | Description                                          |
| ---------------------- | ---------------------------------------------------- |
| `add(Item)`            | Adds an item to the end                              |
| `get(Index)`           | Gets an item at an index                             |
| `swap(Index1, Index2)` | Swaps two items                                      |
| `pop_back()`           | Removes and returns the last item                    |
| `pop(Index)`           | Removes and returns an item without preserving order |
| `getMaxSize()`         | Returns the table's maximum capacity                 |
| `getSize()`            | Returns the current number of items                  |
| `copy(false, Items)`   | Copies items using the calling thread                |
| `copy(true, Items)`    | Copies items using the worker threads                |

---

# Important Notes

### Fixed capacity

`unordered_table` cannot grow beyond its compile-time capacity.

```cpp
unordered_table<int, 100>
```

always has a maximum capacity of `100`.

### `pop()` does not preserve order

Removing an item with `pop()` swaps it with the last element.

This makes removal fast, but changes the ordering of the remaining elements.

### Multithreading requires a synchronizer

If you call:

```cpp
Numbers.copy(true, Items);
```

the table must have been created with a valid `MultithreadingSynchronizer`.

For example:

```cpp
MultithreadingSynchronizer Synchronizer(4);

unordered_table<int, 1000> Numbers(&Synchronizer);
```

Otherwise, `copy()` will throw an error.

### Oversized copies produce warnings

If the input contains more elements than the table can hold, the table copies everything that fits and warns about the excess.

```text
Input:    1000 items
Capacity: 500 items

Copied:   500 items
Warning:  500 items could not be copied
```

---

# Basic Mental Model

Think of `unordered_table` as a fixed-size box:

```text
capacity = 8

┌────┬────┬────┬────┬────┬────┬────┬────┐
│ 10 │ 20 │ 30 │ 40 │    │    │    │    │
└────┴────┴────┴────┴────┴────┴────┴────┘
  ↑                   ↑
 item 0              item 3

size = 4
capacity = 8
```

The table can hold up to `8` items, but currently contains `4`.

Adding an item increases `size`.

Removing an item decreases `size`.

`capacity` never changes.

For multithreaded `copy()`, multiple persistent workers fill different parts of the same array:

```text
Worker 0 ──────► [0] [4]
Worker 1 ──────► [1] [5]
Worker 2 ──────► [2] [6]
Worker 3 ──────► [3] [7]
```

Once all workers finish, `copy()` returns.
