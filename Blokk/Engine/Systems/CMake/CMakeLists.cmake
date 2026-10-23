add_library(Blokk STATIC
    ../../GameObjects/Frames+Animation.cpp
    ../../GameObjects/Position.cpp
    ../../GameObjects/Velocity.cpp
    ../../GameObjects/Visibility.cpp
    ../../GameObjects/GameObject.cpp

    ../IncrementFrames.cpp
    ../Threading.cpp
    ../UserUpdates.cpp
    ../Velocities.cpp
    ../Visibility_Culling/Visibility_AxisCulling.cpp
    ../Visibility_Culling/Visibility_BasicCulling.cpp
    ../Sound.cpp
    ../Sound.hpp

    ../../Math/Constants.hpp
    ../../Math/MathHelpers.hpp
    ../../Math/Geometry/Geometry.hpp
    ../../Math/Matrix/Matrix4.hpp
    ../../Math/Quaternion/Quaternion.hpp
    ../../Math/Quaternion/QuaternionMath.hpp
    ../../Math/SIMD/SSECompat.hpp
    ../../Math/Vector/Vector2.hpp
    ../../Math/Vector/Vector2Math.hpp
    ../../Math/Vector/Vector3.hpp
    ../../Math/Vector/Vector3Math.hpp
    ../../Math/Vector/Vector4.hpp
)

target_include_directories(Blokk
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../Include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../GameObjects
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Helpers
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Math
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Blokk
    PRIVATE
        SDL3::SDL3
        xsimd
        imgui
        BFCL
        OpenAL::OpenAL
)

# SIMD DETECTOR -------------------------------------------------------------

include(CheckCXXCompilerFlag)
include(CheckCXXSymbolExists)

# Get data
include("${CMAKE_CURRENT_LIST_DIR}/Data.cmake")

set(BLOKK_CONFIG_FILE "" CACHE FILEPATH "User custom path to macro configuration file")

# Initialize list collections for final assignment
set(ACCUMULATED_FLAGS "")
set(ACCUMULATED_MACROS "")

if(NOT BLOKK_MACRO_FILE)
    message(STATUS "BlokkForge compile time macro file not specified. Defaulting to auto-detection...")
    
    # Identify active compiler list name for the fallback auto-detector
    if(MSVC)
        set(ACTIVE_COMPILER_FLAGS BLOKK_MSVC_FLAGS)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(ACTIVE_COMPILER_FLAGS BLOKK_GCC_CLANG_FLAGS)
    else()
        message(FATAL_ERROR "BlokkForge: Compiler not supported")
    endif()

    # Loop through the variables cleanly via double-dereferencing ${${ACTIVE_COMPILER_FLAGS}}
    foreach(MACRO_NAME FLAG_VALUE IN ZIP_LISTS BLOKK_ALL_COMPTIME_MACROS ${ACTIVE_COMPILER_FLAGS})

        if("${FLAG_VALUE}" STREQUAL "")
            continue()
        endif()

        message(STATUS "Evaluating macro: ${MACRO_NAME} with flag: ${FLAG_VALUE}")

        # Use unique variable scopes to avoid caching collisions during detection
        check_cxx_compiler_flag("${FLAG_VALUE}" HAS_${MACRO_NAME})

        if(HAS_${MACRO_NAME})
            if(MSVC)
                # MSVC is cascading master switches, keep only the top tier choice
                set(ACCUMULATED_FLAGS "${FLAG_VALUE}")
                set(ACCUMULATED_MACROS "${MACRO_NAME}")
                break()
            else()
                # GCC/Clang can stack multiple active vector extensions simultaneously
                # Some entries hold several space-separated flags, split them into separate arguments
                separate_arguments(FLAG_LIST NATIVE_COMMAND "${FLAG_VALUE}")
                list(APPEND ACCUMULATED_FLAGS ${FLAG_LIST})
                list(APPEND ACCUMULATED_MACROS "${MACRO_NAME}")
            endif()
        endif()

    endforeach()

else()

    if(NOT EXISTS "${BLOKK_MACRO_FILE}")
        message(FATAL_ERROR "BlokkForge: Configuration file not found at: ${BLOKK_MACRO_FILE}")
    endif()

    # Extract the file extension to branch pipelines properly
    get_filename_component(BLOKK_FILE_EXT "${BLOKK_MACRO_FILE}" EXT)

    if("${BLOKK_FILE_EXT}" STREQUAL ".json")
        include("${CMAKE_CURRENT_LIST_DIR}/JSONParser.cmake")
    else()
        # The user passed an invalid file type (not a .h or JSON), throw an error
        message(FATAL_ERROR "BlokkForge: Unsupported file extension target format: ${BLOKK_FILE_EXT}")
    endif()

endif()

# Apply discovered parameters to target
if(NOT "${ACCUMULATED_FLAGS}" STREQUAL "")
    target_compile_options(Blokk PRIVATE ${ACCUMULATED_FLAGS})
    target_compile_definitions(Blokk PRIVATE ${ACCUMULATED_MACROS})
    message(STATUS "Blokk successfully configured with options: ${ACCUMULATED_FLAGS}")
    message(STATUS "Blokk active definition profiles: ${ACCUMULATED_MACROS}")
endif()
