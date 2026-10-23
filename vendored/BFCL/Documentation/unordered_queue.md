# `unordered_queue`

`unordered_queue` is a lightweight container for storing objects through pointers while supporting both **owned** and **borrowed** values.

Unlike a normal queue, `unordered_queue` does **not** preserve ordering when removing an item from the middle. Removal uses a **swap-and-pop** operation, making arbitrary removal very fast.

## Features

* Stores objects through pointers
* Supports both **owned** and **borrowed** objects
* Fast `push()` operations
* Fast arbitrary removal with `remove_at()`
* Fast arbitrary pop operations with `pop_at()`
* `front()` and `back()` access
* Automatic cleanup of owned objects
* `reserve()` support
* Move/copy behavior can be controlled by the `Type`
* Copying the queue itself is disabled

---

## Basic Usage

```cpp
unordered_queue<int> Queue;

Queue.push(10);
Queue.push(20);
Queue.push(30);

std::cout << Queue.front() << '\n'; // 10
std::cout << Queue.back() << '\n';  // 30
```

The queue now contains:

```text
[10] [20] [30]
 ↑           ↑
front       back
```

---

# Ownership

`unordered_queue` has two ways to add objects.

## `push()`

```cpp
Queue.push(value);
```

`push()` creates and owns its own copy of the value.

For example:

```cpp
int Number = 42;

Queue.push(Number);
```

The queue now owns a separate `int`.

Conceptually:

```text
Number = 42

Queue
  |
  v
[42]   <-- Queue owns this
```

Changing `Number` later does not change the value stored in the queue.

### Moving a value

You can also move a value into the queue:

```cpp
std::string Name = "Hello";

Queue.push(std::move(Name));
```

This allows the value to be moved into the queue instead of copied.

---

## `push_ref()`

```cpp
Queue.push_ref(value);
```

`push_ref()` stores a pointer to an existing object.

The queue **does not own** the object.

```cpp
int Number = 42;

Queue.push_ref(Number);
```

Conceptually:

```text
Number = 42
   ^
   |
Queue ----+
          |
          v
       [pointer]
```

The original `Number` remains owned by whatever created it.

### Important

The referenced object must remain alive while it is stored in the queue.

```cpp
{
    int Number = 42;

    Queue.push_ref(Number);
} // Number is destroyed here

// Queue now contains a dangling pointer!
```

Do not allow a borrowed object to be destroyed while the queue still references it.

---

# Adding Items

## `push()`

Adds an owned value to the back.

```cpp
Queue.push(value);
```

## `push_ref()`

Adds a borrowed reference to the back.

```cpp
Queue.push_ref(value);
```

Both operations increase `size()` by one.

---

# Accessing Items

## `operator[]`

Returns a reference to an item.

```cpp
Queue[0] = 100;
```

This allows the stored object to be directly modified.

An exception is thrown when the index is outside the current range.

---

## `operator()`

Returns a pointer to an item.

```cpp
int* Value = Queue(0);

if (Value)
{
    std::cout << *Value;
}
```

This is useful when pointer access is more convenient than reference access.

An exception is thrown when the index is outside the current range.

---

## `front()`

Returns a reference to the first item.

```cpp
int& Value = Queue.front();
```

Throws an exception if the queue is empty.

---

## `back()`

Returns a reference to the last item.

```cpp
int& Value = Queue.back();
```

Throws an exception if the queue is empty.

---

## `data()`

Returns a constant reference to the internal vector of pointers.

```cpp
const std::vector<int*>& Items = Queue.data();
```

This can be useful when code needs direct access to the queue's pointer storage.

The vector itself cannot be modified through this reference.

```cpp
Queue.data().push_back(...); // Not allowed
```

However, the pointed-to objects are still mutable:

```cpp
Queue.data()[0]->some_value = 10;
```

---

# Removing Items

## `pop_front()`

Returns and removes the first item.

```cpp
int Value = Queue.pop_front();
```

For an owned object, the value is moved out before the object is deleted.

For a borrowed object, the value is copied out and the original object is not deleted.

---

## `pop_back()`

Returns and removes the last item.

```cpp
int Value = Queue.pop_back();
```

The same ownership rules as `pop_front()` apply.

---

## `pop_at()`

Returns and removes an item at a specific index.

```cpp
int Value = Queue.pop_at(3);
```

This is different from a normal vector erase operation.

The removed item is replaced by the last item in the queue.

For example:

```text
Before:

[ A ] [ B ] [ C ] [ D ] [ E ]
              ^
           remove_at(2)

After:

[ A ] [ B ] [ E ] [ D ]
```

The order of the remaining elements is therefore **not guaranteed**.

This is what makes the operation fast.

---

## `remove_at()`

Removes an item without returning it.

```cpp
Queue.remove_at(3);
```

Internally, the item is swapped with the last item and then removed.

```text
Before:

[ A ] [ B ] [ C ] [ D ] [ E ]

remove_at(1)

After:

[ A ] [ E ] [ C ] [ D ]
```

This operation is approximately **O(1)** apart from destruction of the removed object.

---

## `remove_all()`

Removes every item from the queue.

```cpp
Queue.remove_all();
```

Owned objects are deleted automatically.

Borrowed objects are simply removed from the queue.

---

# Queue State

## `empty()`

Checks whether the queue contains no items.

```cpp
if (Queue.empty())
{
    // Queue is empty
}
```

Returns:

```cpp
true
```

when the queue contains no items.

---

## `size()`

Returns the number of items currently stored.

```cpp
uint32_t Count = Queue.size();
```

Example:

```cpp
Queue.push(10);
Queue.push(20);
Queue.push(30);

std::cout << Queue.size(); // 3
```

---

## `reserve()`

Pre-allocates space for a number of items.

```cpp
Queue.reserve(1000);
```

This can reduce reallocations when many items are going to be inserted.

For example:

```cpp
unordered_queue<int> Queue;

Queue.reserve(100000);

for (int i = 0; i < 100000; ++i)
{
    Queue.push(i);
}
```

---

# Replacing the Contents

## `assign()`

Replaces the queue's current items with the pointers from another vector.

```cpp
std::vector<int*> Items;

Items.push_back(&A);
Items.push_back(&B);
Items.push_back(&C);

Queue.assign(Items);
```

The pointers are copied into the queue.

The objects are treated as **borrowed**.

That means the queue will not delete them.

```text
Items
  |
  +----> A
  |
  +----> B
  |
  +----> C

Queue
  |
  +----> A
  |
  +----> B
  |
  +----> C
```

The objects must remain alive while the queue references them.

---

# Removal and Ownership

The queue stores an ownership flag alongside every pointer.

Conceptually:

```text
Items:  [ptr A] [ptr B] [ptr C]
owned:  [ true] [false] [ true]
```

This means:

```text
A -> Queue owns it
B -> Queue borrows it
C -> Queue owns it
```

When an item is removed:

* Owned objects are deleted.
* Borrowed objects are not deleted.

This allows the same container to safely store both types of object.

---

# Automatic Cleanup

When an `unordered_queue` is destroyed:

```cpp
~unordered_queue()
{
    remove_all();
}
```

All owned objects are automatically deleted.

Borrowed objects are left untouched.

Example:

```cpp
int ExternalValue = 10;

{
    unordered_queue<int> Queue;

    Queue.push(20);
    Queue.push_ref(ExternalValue);
}
```

When `Queue` is destroyed:

```text
20 -> deleted
10 -> NOT deleted
```

`ExternalValue` is still owned by the caller.

---

# Unordered Behavior

The most important difference between `unordered_queue` and a normal queue/vector is that removing an arbitrary item does **not** preserve order.

For example:

```text
[A] [B] [C] [D] [E]
```

Calling:

```cpp
Queue.remove_at(1);
```

may produce:

```text
[A] [E] [C] [D]
```

instead of:

```text
[A] [C] [D] [E]
```

This is intentional.

The container is designed for situations where **fast removal matters more than maintaining element order**.

---

# Complexity

| Operation      |                    Complexity |
| -------------- | ----------------------------: |
| `push()`       |                Amortized O(1) |
| `push_ref()`   |                Amortized O(1) |
| `front()`      |                          O(1) |
| `back()`       |                          O(1) |
| `operator[]`   |                          O(1) |
| `operator()`   |                          O(1) |
| `pop_front()`  |                          O(1) |
| `pop_back()`   |                          O(1) |
| `pop_at()`     |                          O(1) |
| `remove_at()`  |                          O(1) |
| `empty()`      |                          O(1) |
| `size()`       |                          O(1) |
| `reserve()`    | O(n) when reallocation occurs |
| `remove_all()` |                          O(n) |

The O(1) removal operations are possible because the container uses **swap-and-pop**.

---

# Copying

Copying an `unordered_queue` is disabled.

```cpp
unordered_queue<int> A;
unordered_queue<int> B = A; // Error
```

This is intentional because the queue stores raw pointers and owns some of the objects they point to.

Allowing a normal shallow copy could result in multiple queues trying to delete the same object.

---

# Example

```cpp
#include <iostream>

unordered_queue<int> Queue;

int ExternalValue = 100;

// Owned values
Queue.push(10);
Queue.push(20);

// Borrowed value
Queue.push_ref(ExternalValue);

std::cout << Queue.front() << '\n';
std::cout << Queue.back() << '\n';

Queue[0] = 50;

std::cout << Queue.size() << '\n';

Queue.pop_at(1);

Queue.remove_at(0);

Queue.remove_all();
```

---

# When Should I Use `unordered_queue`?

`unordered_queue` is useful when:

* You need very fast arbitrary removal.
* You do not care about element ordering.
* You want to store pointers to existing objects.
* You sometimes want the queue to own objects.
* You are building systems where objects are frequently added and removed.

It can be particularly useful for engine systems such as:

```text
Entity storage
Component collections
Active objects
Task/job lists
Particle systems
Collision objects
Visibility lists
```

---

# Function Reference

| Function            | Purpose                               |
| ------------------- | ------------------------------------- |
| `push(value)`       | Add an owned value                    |
| `push_ref(value)`   | Add a borrowed value                  |
| `front()`           | Access the first item                 |
| `back()`            | Access the last item                  |
| `pop_front()`       | Return and remove the first item      |
| `pop_back()`        | Return and remove the last item       |
| `pop_at(index)`     | Return and remove an item at an index |
| `remove_at(index)`  | Remove an item at an index            |
| `remove_all()`      | Remove every item                     |
| `operator[](index)` | Access an item by reference           |
| `operator()(index)` | Access an item by pointer             |
| `data()`            | Access the internal pointer array     |
| `assign(vector)`    | Replace the queue's contents          |
| `reserve(count)`    | Reserve storage                       |
| `empty()`           | Check whether the queue is empty      |
| `size()`            | Get the number of items               |

---

## Summary

`unordered_queue` is essentially a **pointer-based, unordered container with optional ownership**.

Its main design goal is simple:

```text
Fast insertion
      +
Fast removal
      +
Optional ownership
      =
unordered_queue
```

The trade-off is that removing arbitrary elements can change the order of the remaining elements.

Use it when **performance and fast removal are more important than preserving order**.
