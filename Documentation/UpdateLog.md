# v0.1.9
> not completed yet

### Fixes
* Made `SetTargetExecutionTime(double)` public inside the `EngineCore` class instead of inside the `Diagnostics` class
* Implemented `SetThreadCount(uint32_t)` - was declared but not defined

### CMake 
* Changed configuration macros to JSON configuration
* Removed macro logic outside of EngineClassData.hpp and into CMake
* Moved CMake into CMake/ directory and split into 3 files for organization (Data, CMakeLists, JSONParser)
* Updated documentation for threading systems and CMake build commands
* Made compiler support guardrails to SIMD types

# v0.1.8
> 9/6/2026

## Cross-Platform Compatibility

Blokk v0.1.8 is focused on making the engine more portable, resilient, and capable of taking advantage of different CPU architectures.

This update introduces **NEON support**, improves SIMD detection and fallback behavior, and strengthens platform-specific guardrails throughout the engine.

### SIMD & CPU Support

* Added **ARM NEON support** for compatible ARM64 platforms.
* Added **scalar implementations** for systems where supported SIMD instructions are unavailable.
* Unsupported SIMD hardware no longer causes an error; Blokk can fall back to scalar implementations instead.
* Added dedicated SIMD branches for different instruction sets, including SSE2 and NEON.
* Improved SIMD detection and architecture-specific handling.
* Added necessary architecture guards, declarations, and includes.
* Added NEON implementations to relevant engine systems, including:

  * Visibility / render culling
  * Velocity processing
  * Animation processing
  * Diagnostics

Blokk now treats SIMD as an optimization rather than a requirement, allowing the engine to remain functional across a wider range of hardware.

## Threading Improvements

The threading system has received a significant architectural update.

* Added persistent worker threads.
* Worker threads are created once and reused instead of repeatedly being destroyed and recreated.
* Added thread control functions.
* Improved handling of changing workloads.
* Added proper cleanup when the engine is destroyed.
* Reduced unnecessary thread-management overhead.

These changes provide a stronger foundation for future parallel engine systems and help avoid the cost of repeatedly creating and destroying threads.

## Rendering & Camera

* Added camera support to basic render culling.
* Improved visibility handling with architecture-specific implementations.
* Added additional guardrails around rendering-related functionality.
* Improved the separation between rendering-enabled and rendering-disabled configurations.

## Engine Reliability

* Added additional include guards and required includes.
* Added platform-specific guardrails to declarations and definitions.
* Fixed variable-access bugs.
* Removed duplicated camera state.
* Condensed and cleaned up several implementations.
* Improved behavior when optional engine features are disabled.

## Summary

**v0.1.8 is a cross-platform compatibility and architecture-focused release.**

The goal of this update is to make Blokk more adaptable to different hardware while maintaining a reliable fallback when hardware-specific optimizations aren't available.

With **ARM NEON support, scalar SIMD fallbacks, improved SIMD detection, persistent worker threads, and stronger platform guardrails**, Blokk is better prepared to run across different architectures and serve as a foundation for future performance improvements.


# v0.1.7
> 9/3/2026

## Rendering

* Added the new SDL3-based rendering system.
* Added SDL_image support for loading textures from image files.
* Added support for loading individual image files as animation frames.
* Added `Renderer` animation functions:

  * `CreateNewEmptyAnimation()`
  * `AddFrameToAnimation()`
  * `CreateAnimation()`
* Added texture and per-frame dimension storage to `EngineCore`.
* Added rendering object index tracking.

## Camera

* Added `CameraController`.
* Added camera position controls:

  * `SetPosition()`
  * `SetXPosition()`
  * `SetYPosition()`
  * `ChangeXPosition()`
  * `ChangeYPosition()`
* Added support for accessing the camera through the `EngineCore`.

## Engine Architecture

* Reworked animation functionality to separate rendering functions from animation data.
* Updated `EngineCore` to support the new rendering architecture.
* Added conditional rendering support through `Blokk_Rendering_Enabled`.
* Updated engine CMake configuration for SDL3_image.
* Added SDL_image as a vendored Git submodule.

## Bug Fixes

* Fixed issues with rendering-related diagnostics when rendering is disabled.
* Fixed animation system organization and separated animation operations from stored animation data.
* Fixed build configuration issues related to SDL and SDL_image integration.
* Fixed camera state organization so camera position is managed by the camera controller.
* Fixed various engine-system integration issues encountered during the rendering overhaul.

## Dependencies

* Added **SDL_image** and its required submodules.
* Updated the project's `.gitmodules` configuration.


# v0.1.6
> 8/31/2026

## Update Notes

### Major Changes

* **Removed the internal field update command queues**

  * Object data is now updated directly instead of being passed through update queues.
  * Removed the associated queue variables, functions, and processing implementations.
  * Simplifies the engine's internal update system and reduces unnecessary intermediate work.

* **Reworked object destruction**

  * Replaced the previous `DestroyObject` implementation with separate:

    * `DestroyDynamicObject`
    * `DestroyStaticObject`
  * Improved handling of objects stored in static and dynamic data.
  * Fixed object data and index handling when objects are moved between storage types.

* **Improved static/dynamic object management**

  * Fixed logic for switching objects between static and dynamic storage.
  * Added visibility handling to object swapping and creation operations.
  * Fixed a bug where creating a dynamic object when no static objects existed could leave its internal index incorrect.

### Visibility

* Added a new visibility implementation with direct engine data manipulation.
* Added visibility support to object creation parameters.
* Added a `Visible` field to `ObjectCreationParams`.
* Updated visibility-related object operations to work with the new direct data system.

### Engine Internals

* Renamed the internal `EngineObjects` variable to `Engine`.
* Changed its type from `size_t` to `uint32_t`.
* Added a destructor to improve cleanup when the engine/project is destroyed.
* Added the `IndexRange` structure to the `Blokk` namespace.
* Made `IndexRange::GetSize()` `const`.

### Threading

* Adjusted thread initialization so the worker's static `EngineCore` reference is set **before** opening the worker thread.
* This fixes initialization ordering and makes the worker setup more reliable.

### Rendering

* Fixed a rendering offset issue where positions were being offset in the wrong direction.

## Cleanup

* Removed obsolete update queue implementations.
* Removed old field update command processing.
* Consolidated duplicate `ProcessAddCommand` implementations.
* Removed internal functions that were only required by the previous queue-based update system.

Overall, **v0.1.6 simplifies Blokk's internal data flow, improves object management, and lays the groundwork for more direct and efficient engine updates.**


# v0.1.5
> 8/30/2026

## New Features
- Added configurable thread control with `Blokk_Thread_Control`.
- Added `Blokk_Thread_AdaptiveTiming` for automatically adjusting the number of worker threads based on frame execution time.
- Added `Blokk_Thread_FixedCount` for manually controlling the number of worker threads.
- Added `SetThreadCount()` to change the active worker thread count.
- Added safety checks for invalid thread counts.
- Improved SIMD visibility culling organization with internal helper functions.
- Added improved namespace organization for internal engine functionality.

## Visibility Culling
- Refactored Basic and Axis visibility culling.
- Organized SSE2, AVX2, and AVX-512 visibility functions under `InternalHelpers`.
- Fixed visibility range indexing when processing worker ranges.

## Threading
- The engine can now choose between adaptive and fixed thread-count modes.
- Adaptive threading can increase or decrease the number of active worker threads based on performance.
- Fixed threading allows the user to specify exactly how many worker threads the engine should use.

## V0.1.4
> 8/28/2026

* Cleaned up several parts of the engine code and removed unused update queues.

### Added:

* Camera support with X and Y movement.
* Camera-aware visibility culling across SSE2, AVX2, and AVX-512.

### Fixed:

* Object creation and destruction.
* Queued update commands not being removed after processing.

### Improved:

* Handling of static and dynamic objects when creating, destroying, and swapping them.
* Some diagnostics getters and general code consistency.

### Summary
This update mainly focuses on making object management more reliable while laying the groundwork for camera support.

> **PLEASE NOTE:**
> This update's new camera **does not work with basic culling**; you'll have to enable visibility culling to use the camera

## V0.1.3
> 8/24/2026

## Diagnostics & Architecture Update

### Added
- Added engine diagnostics through `PrintDiagnostics()`.
- Added getters for:
  - Total object count
  - Static object count
  - Dynamic object count
  - Opened thread count
  - Total thread count
  - Previous frame execution time
  - Target execution time
  - SIMD register size
- Added the ability to set the target engine execution time.
- Added a basic example project template.

### Changed
- Separated engine updating and rendering into separate operations.
- `EngineProcess()` is being replaced with separate update and render stages.
- Reorganized example projects and documentation into dedicated directories.
- Improved the public engine API for easier integration into projects.

### Performance
- Continued optimization of hot engine loops.
- Reduced unnecessary runtime branching in engine systems.
- Improved diagnostics for identifying engine performance bottlenecks.

### Documentation
- Added and reorganized documentation for the engine.
- Added a basic example project showing the minimum setup required to use Blokk.

## V 0.1.1:
> 8/22/2026


### Added

- New rendering culling system: Axis Culling
- Added `AnimFrameCounts` to track animation frame counts
- Added documentation for:
  - Requirements
  - Visibility culling systems

### Changed

- Updated SIMD level detection and function selection
- Split visibility culling into Basic Culling and Axis Culling
- Changed `IndexRange` and related size/index values from `size_t` to `uint32_t`
- Updated CMake to include the new visibility culling files
- Renamed visibility-related files to be more specific

### Performance

- Eliminated switch statements from hot loops in visibility and velocity processing
- Added SIMD-based dispatch for the selected SIMD level
- Improved animation frame handling

### Removed

- Removed AVX support
- Removed the incomplete Blokk-C transpiler
- Removed NEON support for now
