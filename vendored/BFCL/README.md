# BFCL

**BFCL (BlokkForge Core Library)** is a C++ library focused on high-performance data processing, with SIMD, multithreading, and optimized data structures built for handling large amounts of data efficiently.

It is designed to provide **fast, reusable building blocks** for performance-focused applications while keeping the API simple and approachable.

> **BFCL is currently in very early development.** Expect major changes as the library continues to evolve.

## Current Features:

BFCL features custom data structures:
* [`unordered_queue`](Documentation/unordered_queue.md) - A queue optimized for fast removals
* [`coordinate_tree`](Documentation/coordinate_tree.md) - A optimized tree that organizes data into groups based on their position in a 2D space
* [`unordered_table`](Documentation/table.md) - An optimized table with multithreading and a fixed max capacity
