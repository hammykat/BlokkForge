# `concurrent_unordered_queue`

`concurrent_unordered_queue` is a thread-safe version of `unordered_queue`. It allows multiple threads to access and modify the same queue using a mutex to synchronize operations.

The main reason for having this class is to make it easier to share queues between threads without having to manually lock the queue every time an operation is performed.

One use case is **work stealing**, where each worker has its own queue of tasks and can take tasks from other workers when its own queue runs out of work.

## Features

* Thread-safe queue operations using a mutex.
* Add owned or borrowed items.
* Remove items from the front, back, or a specific index.
* `try_pop_front()` and `try_pop_back()` for safely attempting to remove an item without throwing an error when the queue is empty.
* Automatic cleanup of items owned by the queue.
* No requirement to preserve item order when removing items.

## Usage

### Creating a queue

```cpp
concurrent_unordered_queue<IndexRange> ranges;
```

Each queue can store any type that supports the operations required by the functions being used.

### Adding items

Use `push()` to add an item that the queue will own.

```cpp
ranges.push(IndexRange(0, 100));
```

The queue stores its own dynamically allocated object and is responsible for deleting it when it is removed or when the queue is destroyed.

Use `push_ref()` to add a reference to an existing object instead.

```cpp
IndexRange range(0, 100);
ranges.push_ref(range);
```

The queue does not own or delete borrowed objects. The original object must remain alive for as long as the queue needs to access it.

### Removing items

The queue provides several functions for removing items:

| Function         | Description                                          |
| ---------------- | ---------------------------------------------------- |
| `pop_front()`    | Removes and returns the first item.                  |
| `pop_back()`     | Removes and returns the last item.                   |
| `pop_at(idx)`    | Removes and returns the item at the specified index. |
| `remove_at(idx)` | Removes an item without returning it.                |
| `remove_all()`   | Removes all items from the queue.                    |

The `pop_*()` functions throw an error if the requested item does not exist. Use the `try_pop_*()` functions when the queue might be empty.

### Using `try_pop_front()` and `try_pop_back()`

These functions attempt to remove an item and store it in a variable supplied by the caller.

They return `true` if an item was successfully removed or `false` if the queue was empty.

```cpp
IndexRange range;

if (ranges.try_pop_front(range))
{
    ProcessRange(range);
}
```

The queue is locked while checking for an item and removing it. This makes the operation safe against other threads attempting to modify the same queue at the same time.

This is particularly useful for work stealing, because a worker can attempt to steal a range without needing to check `empty()` and then call `pop_back()` separately.

For example:

```cpp
IndexRange range;

if (ThreadRanges[victimIdx].try_pop_back(range))
{
    ProcessRange(range);
}
```

The worker processes the stolen range after the function returns, so it does not hold the queue's mutex while doing the actual work.

## Other functions

| Function          | Description                                                                                                  |
| ----------------- | ------------------------------------------------------------------------------------------------------------ |
| `empty()`         | Returns whether the queue is empty.                                                                          |
| `size()`          | Returns the number of items currently in the queue.                                                          |
| `front()`         | Returns a reference to the first item.                                                                       |
| `back()`          | Returns a reference to the last item.                                                                        |
| `operator[](idx)` | Returns a reference to an item at the specified index.                                                       |
| `operator()(idx)` | Returns a pointer to an item at the specified index.                                                         |
| `data()`          | Returns a reference to the internal item pointer array.                                                      |
| `assign(Target)`  | Replaces the queue's contents with pointers from another vector. The assigned items are treated as borrowed. |
| `reserve(count)`  | Reserves storage for a specified number of items.                                                            |

Individual calls to these functions lock the queue while accessing its internal state. However, functions that return references or pointers cannot keep the mutex locked while the caller uses the returned value. See the thread-safety notes below.

## How the queue works

The queue stores pointers to its items in a `std::vector<Type*>`. A separate array tracks whether each item is owned by the queue or borrowed from somewhere else.

When an item is removed, its pointer is swapped with the last pointer in the array before the last element is removed. This avoids shifting every remaining item, but **the order of the items is not preserved**.

Each queue has its own mutex. Operations that access or modify the queue's internal storage acquire this mutex, preventing multiple threads from modifying the underlying arrays simultaneously.

The mutex is released automatically when the locking object's scope ends.

## Thread-safety notes

The mutex protects the queue's internal container operations. It does not automatically make every object accessed through the queue safe to use from multiple threads.

* **Returned references and pointers:** `operator[]`, `operator()`, `front()`, `back()`, and `data()` return references or pointers after the mutex is released. Using these while another thread modifies the queue can be unsafe.
* **Queue lifetime:** Do not destroy the queue while other threads are still accessing it. Stop and join the worker threads first.
* **Borrowed items:** Objects added with `push_ref()` must remain alive, and any concurrent access to those objects must be synchronized separately when necessary.
* **Checking the queue:** `empty()` and `size()` are safe individual operations, but their results can become outdated immediately after they return. Use `try_pop_front()` or `try_pop_back()` when checking and removing an item must happen as one operation.
* **Processing items:** Keep the time spent holding the mutex short. Remove an item first, then process it after the function returns.

## Limitations

`concurrent_unordered_queue` is not a lock-free queue. Its operations use a mutex, so a thread may have to wait for another thread to finish an operation.

The purpose of this implementation is to provide a straightforward, thread-safe way to share queue operations between workers. Whether it improves performance depends on how often workers contend for the same queue and how much work each item requires.

For the BlokkForge work-stealing system, each worker should normally take work from its own queue first and attempt to steal from other queues when necessary. The range should be removed while holding the mutex, but the actual update or processing should happen outside the locked section.
