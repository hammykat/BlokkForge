# Blokk

A beginner-friendly, high-performance open-source **2D C++ game engine**, designed to provide a simple API while still giving developers direct access to a performance-oriented engine.

Blokk focuses on **high performance, lightweight architecture, and ease of use**, while allowing developers to work directly with C++.

## Features

Blokk is designed around:

* **Beginner-friendly API** — Easy-to-use functions and systems designed to make the engine easy to use and learn.
* **High Performance** — Uses techniques such as SIMD, Structure of Arrays (SoA), cache-friendly data layouts, and parallel processing.
* **Adaptive Threading** — The engine can automatically adjust its worker-thread usage based on execution time.
* **2D Focused** — Built specifically around 2D game development rather than trying to cover every type of game.
* **C++** — Full access to C++ and the underlying engine.
* **SDL3 Rendering** — Uses SDL3 for rendering and SDL_image for loading image assets.
* **Lightweight** — Designed to keep the engine's core systems relatively small and focused.
* **Open Source** — Anyone can inspect the source code, learn from it, contribute improvements, or experiment with the architecture.
* **Control** – Most systems can be manually controlled and on default, are turned off. This allows the user **only pay for what they need** and not anything extra they aren't using.

The goal is to provide a simple API for beginners without sacrificing the performance and control expected from a C++ engine.

## Performance

Blokk is designed around **data-oriented and performance-conscious programming techniques**.

Some of the approaches used by the engine include:

* Data-oriented design
* Structure of Arrays (SoA)
* Cache-friendly data layouts
* SIMD processing
* Parallel task processing
* Adaptive thread management
* Separation of static and dynamic object data
* Visibility culling

The long-term goal is to make Blokk capable of efficiently handling very large numbers of objects while maintaining stable frame times.

Performance is an ongoing area of development, and future versions will include benchmarking and better optimization.

For more information about the engine's architecture, see the [engine architecture documentation](Documentation/EngineArchitecture.md).

## Rendering

Blokk currently uses **SDL3** as its rendering backend, with **SDL_image** for loading image assets.

The rendering system supports:

* Loading image files as animation frames
* Creating and managing animations
* Per-frame dimensions
* Rendering visible objects
* Camera-relative rendering
* Rendering diagnostics
* Optional rendering through `Blokk_Rendering_Enabled`

Rendering is designed to remain separate from the engine's core object-processing systems where possible.

## Threading

Blokk includes configurable thread-management systems.

### Adaptive Threading

With adaptive threading enabled, Blokk monitors frame execution time and can adjust the number of worker threads being used.

This allows the engine to adapt its workload to different hardware rather than requiring a single fixed thread count.

### Fixed Threading

Developers can also configure a fixed number of worker threads when predictable thread usage is preferred.

Thread control can be configured through the engine's configuration macros.

## Visibility Culling

Blokk includes multiple visibility-culling implementations designed to avoid processing objects that are outside the relevant screen area.

Currently available culling approaches include:

* Basic Culling
* Axis Culling

The engine can select optimized implementations based on the available SIMD instruction set.

## Camera

Blokk includes an optional camera system for 2D projects.

The camera supports:

* Setting its position
* Changing its X and Y position
* Accessing camera position through the `ObjectManager`
* Camera-aware rendering
* Camera-aware visibility culling (Basic + Axis)

## Documentation

For complete documentation, see the [documentation](Documentation).

The documentation covers:

* Engine architecture
* Configuration options
* Threading
* Visibility culling
* Camera functionality
* Using Blokk
* Engine systems and internals

## Contributing

Blokk is open source and contributions are welcome.

You can contribute through:

* Code
* Testing
* Documentation
* Bug fixes
* Code review
* UI and tooling
* Rendering
* Performance improvements
* Ideas and suggestions
* Example projects and tutorials

If you want to contribute, see the project's contribution guidelines in [`CONTRIBUTING.md`](CONTRIBUTING.md).

## Requirements

| Component            | Requirement                         |
| -------------------- | ----------------------------------- |
| **CPU Architecture** | x86-64 or ARM64                     |
| **SIMD**             | Optional optimization               |
| **SSE2**             | Supported                           |
| **AVX2**             | Supported                           |
| **AVX-512**          | Supported                           |
| **NEON / ARM**       | Supported on ARM64                  |
| **Scalar Fallback**  | Supported on all platforms          |
| **OS**               | Windows, Linux, macOS*              |
| **RAM**              | 4 GB recommended minimum            |
| **GPU**              | SDL3-compatible graphics hardware** |
| **C++**              | C++20                               |
| **Build System**     | CMake                               |
| **Compiler**         | MSVC, GCC, or Clang                 |

* Windows x86-64 is currently the primary tested platform. Linux x86-64, macOS x86-64, macOS ARM64, Linux ARM64, and Windows ARM64 are currently untested.

** Required when using Blokk's built-in rendering system.

## License

Blokk is licensed under the **zlib License**.

See [`LICENSE`](LICENSE) for the full license text.
