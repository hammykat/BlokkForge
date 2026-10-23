# Configurations

These are configuration options you can set to change the engine's behavior and add specific features.
These are all configured inside a JSON file. You need to pass the JSON file path to compile the engine. 

You can do that via a command line switch. The compilation command is: 
`cmake -B (folder_for_build) -S . -DBLOKK_CONFIG_FILE=(file_to_configuration_JSON_file)` and then 
`cmake --build build --config Release`. Remember to replace the text inside the paranthesis with what it actually needs!

Here are some of the configurable options:
* [Camera](#camera)
* [Diagnostics](#diagnostics)
* [Render Culling Systems](#render-culling-systems)
* [Threading Systems](#threading-systems)
* [SIMD preferences](#simd-preferences)

## Setting features

To enable any feature that is either enabled or disabled, you can set it's type to `"true"` (or any of the other JSON true values, such as `1` or `yes`. Note that it is not case sensitive as it converts to uppercase before checking, meaning that you can write `true` or `True` or even `TrUe` and it'll have the same effect). Most features, if not set, are defaulted to `false`, or not enabled.

| Feature        | Macro                                                       | Default  |
| -------------- | ----------------------------------------------------------- | -------- |
| Camera         | `"camera_enabled": "true"`                                  | Disabled |
| Diagnostics    | `"diagnostics": "true"`                                     | Disabled |
| Threading System | `"threading_type": "manual"` OR `"threading_type": "adaptive"` | Adaptive |
| Render Culling Type  | `"render_culling_type": "axis"` OR `"render_culling_type": "basic"` | Axis |


## Camera

The camera is an optional feature that lets you move the view around the game world. When enabled, objects are rendered relative to the camera's position, so moving the camera changes which part of the world is shown on screen.

You can set or change the camera's position using these functions:

| Function                                  | Use                                         |
| ----------------------------------------- | ------------------------------------------- |
| `void SetPosition(int32_t X, int32_t Y);` | Set the camera's position                   |
| `void SetPosition(Vector2 Pos);`          | Set the camera's position using a `Vector2` |
| `void SetXPosition(int32_t X);`           | Set the camera's X position                 |
| `void SetYPosition(int32_t Y);`           | Set the camera's Y position                 |
| `void ChangeXPosition(int32_t X);`        | Move the camera along the X axis            |
| `void ChangeYPosition(int32_t Y);`        | Move the camera along the Y axis            |

The camera position represents the point the view is offset from in the world. For example, increasing the camera's X position moves the view toward the right side of the world.

## Diagnostics

The engine has a diagnostics feature, which you can enable with `#define Blokk_Diagnostics`.
Here, you can get the engine's diagnostics, by using the functions it provides you with:

> Please note that all timings are in **milliseconds**

| Function | Use
|----------|----
| `uint32_t GetTotalObjects();` | Get the total object count
| `uint32_t GetStaticObjectCount();` | Get the static object count
| `uint32_t GetDynamicObjectCount();` | Get the dynamic object count
| `uint32_t GetOpenedThreads();` | Get the number of threads currently opened by the engine
| `uint32_t GetTotalThreads();` | Get the total number of threads the engine can use
| `double GetFrameExecutionTime();` | Get the total time the engine took to process the previous frame
| `double GetTargetExecutionTime();` | Get the engine's target execution time for all its processes in a frame
| `uint32_t GetFPS();` | Get the engine's FPS it was initialized with
| `double GetUserUpdateTime();` | Get the amount of time it took to update the user's update commands
| `double GetVelocityTime();` | Get the amount of time the engine took to dynamic object's positions using their velocities
| `double GetAnimationIncrementTime();` | Get the amount of time the engine took to increment all object's frames for animation
| `double GetVisibilityCullingTime();` | Get the amount of time the engine took to cull objects that are going to get rendered
| `double GetRenderTime();` | Get the amount of time the engine took to render the on-screen objects
| `uint32_t GetSIMDRegisterSize();` | Get the engine's detected SIMD register size
| `std::string GetSIMDRegisterType();` | Get the engine's detected SIMD register type (look below)

It also has a function called:
`void SetTargetExecutionTime(double Time);`
Use this function to set the target time the engine will try to execute all it's processes in.

Quite a useful function is:
`void PrintDiagnostics();`
Using this function will print a diagnostics report containing the engine's:

**Frame:**
* FPS
* Total frame time
* Frame execution time
* Target execution time

**Threads:**
* Opened # of threads
* Total sensed thread count
* If the optimal thread count has been reached

**SIMD:**
* SIMD register size (in bits)
* SIMD register type

**System Timing:**
* User update processing
* Positions with Velocities
* Animation frame incrementing
* Visibility / Rendering culling
* Rendering

For example:
```
========== Blokk Diagnostics ==========

Frame
FPS: 144
Frame Time: 6.91 ms
Execution Time: 2.43 ms
Target Execution Time: 6.94 ms

Threads
Opened: 7
Available: 8
Optimal: Yes

SIMD
Register Size: 128 bits
Register Type: SSE2

System Timing
User Updates: 0.42 ms
Velocity: 0.31 ms
Animation: 0.08 ms
Culling: 0.21 ms
Rendering: 0.73 ms
```

## Render culling systems

There are currently 2 culling systems that you can choose the engine to use for rendering:

### 1. Basic Culling

Here, the engine compares every object's x and y to check if they are in the screen.
You need to define:
```cpp
#define Blokk_Render_CullingType 0 // Basic
```

#### Good situations:
* A few hundred objects
* Objects are spread fairly evenly
* Objects don't have wildly different sizes
* You want the simplest culling implementation
* You care more about low overhead than squeezing out every last operation

### 2. Axis Culling

In this, the engine will check each object's y position, filter out all the objects that can't be inside the screen based off of the results, then check the remaining x positions. This can let it perform almost 2x less work than basic culling.
You need to define:
```cpp
#define Blokk_Render_CullingType 0 // Basic
```

**Best for:** large scenes, large worlds, and situations where many objects can be quickly rejected by their X or Y position.

#### Good situations
- Thousands or more objects
- Large worlds with a relatively small viewport
- Objects spread across a large area
- Many objects are completely outside the screen
- Objects have predictable or relatively simple bounding dimensions
- Visibility checks are a major part of frame time
- You want to take advantage of SIMD processing
- You need high-throughput culling for large object counts

### Example

A large world might contain:

```
100,000 objects  
↓  
X-axis culling  
↓  
20,000 candidates  
↓  
Y-axis culling  
↓  
4,000 visible objects  
↓  
Render
```

AxisCulling is especially useful when most objects are far outside the viewport, because objects can be rejected without performing the complete visibility check.

## Threading Systems

Blokk has 2 threading systems:
* Adaptive
* Manual

The default is adaptive timing.

### 1. Adaptive

The adaptive timing systems lets the engine handle the threads and adapts to efficiently handle the amount of work the engine has to complete by timing the time it takes to execute.

The system first **opens all available threads at the start of the program**. It'll **control the emount of threads used, not the amount of threads open**, because thread creation and destruction is expensive, to avoid lag or stuttering. 

The engine will time all the systems and see if the time taken to execute all the systems is greater than the amount of time it is targeting (defaulting to 80% of the frame time but can be change using `SetTargetExecutionTime(double);`)

If the time taken to execute all it's systems is greater than the target time, it'll **start using another thread and then time it again. If the time was worse than last time, it'll stop using that thread**. It keeps track of how many threads it's currently using using a counter.

### 2. Manual

The manual threading means that the user is responsible for manually telling the engine to create or destroy threads. It doesn't effect how the engine actually uses those threads.

Here are the basic functions:

| Function | Use
|----------|-----
|`OpenThread();` | Open a thread for the engine to use
|`DestroyThread();` | Destroy one of the engine's threads
|`SetThreadCount();` | Set the engine's thread count

> **NOTE:**
> Avoid creating or destroying threads every frame as it can be expensive. Trying to set the thread count to an invalid number (such as 0 or greater than the hardware thread count) will result in an error.

## SIMD Preferences

The engine uses SIMD to process multiple pieces of data at once under the hood to speed up execution time. The engine will automatically select the correct SIMD type **at runtime**, ensuring multiple different versions don't have to be compiled to support every type.

While the engine does select the best supported type at runtime, the user can choose which types they want to support. You can do this by giving the engine the types of SIMD it should check for. The engine currently supports:

### Using MSVC

| JSON Configuration Type
| -------------------------
| "avx512vnni"
| "avx512vbmi2"
| "avx512vbmi"
| "avx512ifma"
| "avx512"
| "avxvnni"
| "avx2"
| "avx"
| "sse4_2"
| "sse4_1"
| "ssse3"
| "sse3"
| "sse2"

### Using GCC / Clang

| JSON Configuration Type
|-----------------------
| "avx512vnni"
| "avx512vbmi2"
| "avx512vbmi"
| "avx512ifma"
| "avx512"
| "avxvnni"
| "avx2"
| "avx"
| "sse4_2"
| "sse4_1"
| "ssse3"
| "sse3"
| "sse2"
| "i8mm"
| "neon64"
| "neon"
| "sve512"
| "sve256"
| "sve128"
| "rvv512"
| "rvv256"
| "rvv128"
| "vsx"
| "vxe"
| "wasm"

### Putting them into the JSON

With all your types, now you can put them into your JSON. You need to define `simd_preferences` and then give a list of all the type you want. 

For example:
```json
"simd_preferences": [
	"avx2",
	"avx",
	"sse3",
	"sse2"

	# And then any more you would want
],

# Other configurations

```

**Make sure to type them exactly as in the tables above, or else it'll throw an error.**
