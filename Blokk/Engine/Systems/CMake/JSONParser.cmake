message(STATUS "BlokkForge: Parsing user configuration from JSON file...")

# Read file and store    
file(READ "${BLOKK_MACRO_FILE}" JSON_DATA)

# Get length and last index
string(JSON ARRAY_LENGTH LENGTH "${JSON_DATA}" "simd_preferences")
math(EXPR LOOP_MAX "${ARRAY_LENGTH} - 1")

# Loop over items
foreach(INDEX RANGE ${LOOP_MAX})
    string(JSON ARCH_PREF GET "${JSON_DATA}" "simd_preferences" ${INDEX})

    # Find the item in the JSON types list
    list(FIND BLOKK_JSON_TYPES "${ARCH_PREF}" ELEMENT_INDEX)
    if(${ELEMENT_INDEX} EQUAL -1) # If not present in the lists, throw an error
        message(FATAL_ERROR "BlokkForge: Invalid SIMD type passed in JSON configurations: (${ARCH_PREF})")
    endif()

    # Get the item
    list(GET BLOKK_ALL_COMPTIME_MACROS ${ELEMENT_INDEX} MATCHING_MACRO)

    # Get the matching flag from the correct list    
    if(MSVC)
        list(GET BLOKK_MSVC_FLAGS ${ELEMENT_INDEX} MATCHING_FLAG)
    else()
        list(GET BLOKK_GCC_CLANG_FLAGS ${ELEMENT_INDEX} MATCHING_FLAG)
    endif()

    # If the engine sees that the compiler profile doesn't support the platform target
    if("${MATCHING_FLAG}" STREQUAL "")
        message(FATAL_ERROR "BlokkForge: SIMD type '${ARCH_PREF}' is not supported by this engine platform profile") 
    endif()

    # Check if the compiler supports the flag
    check_cxx_compiler_flag("${MATCHING_FLAG}" JSON_HAS_${MATCHING_MACRO})
        
    if(JSON_HAS_${MATCHING_MACRO})
        if(MSVC)
            set(ACCUMULATED_FLAGS "${MATCHING_FLAG}")
            set(ACCUMULATED_MACROS "${MATCHING_MACRO}")
            break()
        else()
            # Modifying parent lists directly with zero friction!
            list(APPEND ACCUMULATED_FLAGS "${MATCHING_FLAG}")
            list(APPEND ACCUMULATED_MACROS "${MATCHING_MACRO}")
        endif()
    else()
        # Cleanly warn the user that the hardware can't run it and let the loop check the next fallbacks
        message(STATUS "BlokkForge: SIMD type '${ARCH_PREF}' is valid but not supported by the host compiler/hardware. Skipping...")
    endif()
endforeach()

# Other proprties -----------------------------------------------

# Rendering enabled
set(RENDERING_ACTIVE FALSE) # Local variable, used for cull type to ignore it if rendering is not enabled
string(JSON RENDERING_ENABLED ERROR JSON_ERR GET "${JSON_DATA}" "rendering_enabled")
if("${JSON_ERR}" STREQUAL "NOTFOUND") # No error, the user has defined
    string(TOUPPER "${RENDERING_ENABLED}" RENDERING_ENABLED_UPPER) # Convert to uppercase
    if(RENDERING_ENABLED_UPPER) # User set to true
        message("Rendering set to enabled")
        target_compile_definitions(Blokk PRIVATE Blokk_Rendering_Enabled) # Define rendering enabled
        set(RENDERING_ACTIVE TRUE) # Set local variable
    else()
        message("Rendering set to disabled")
    endif()
else()
    message("Rendering enabled not set. Deafulting to: false")
endif()

# Render cull type (Basic or Axis)
if(RENDERING_ACTIVE) # Ignore if rendering isn't enabled
    string(JSON RENDER_CULLTYPE ERROR JSON_ERR GET "${JSON_DATA}" "rendering_cull_type")
    if("${JSON_ERR}" STREQUAL "NOTFOUND") # No error, the user has defined
        string(TOUPPER "${RENDER_CULLTYPE}" RENDER_CULLTYPE_UPPER) # Convert to uppercase
        if(NOT RENDER_CULLTYPE_UPPER) # User set to 0
            message("Rendering cull type set to 0 (Basic culling)")
            target_compile_definitions(Blokk PRIVATE Blokk_Rendering_CullType=0) # Set cull type as 0 (Basic culling)
        else() # User set to something other than 0
            message("Rendering cull type set to 1 (Axis culling)")
            target_compile_definitions(Blokk PRIVATE Blokk_Rendering_CullType=1) # Set cull type as 1 (Axis culling_
        endif()
    else()
        message("Rendering cull type not set. Deafulting to: 1 (Axis culling)")
        target_compile_definitions(Blokk PRIVATE Blokk_Rendering_CullType=1) # Set cull type as 1 (Axis culling_
    endif()
endif() # RENDERING_ACTIVE

# Threading type (manual or adaptive)
string(JSON THREADING_TYPE ERROR JSON_ERR GET "${JSON_DATA}" "threading_type")
if("${JSON_ERR}" STREQUAL "NOTFOUND") # No error, the user has defined
    string(TOUPPER "${THREADING_TYPE}" THREADING_TYPE_UPPER) # Convert to uppercase
    if(THREADING_TYPE_UPPER STREQUAL "ADAPTIVE") # User set to adaptive
        message("Threading type set to adaptive")
        target_compile_definitions(Blokk PRIVATE Blokk_Threading_AdaptiveTiming) # Define adaptive
    elseif(THREADING_TYPE_UPPER STREQUAL "MANUAL") # User set to manual (they'll control)
        message("Threading type set to manual")
        target_compile_definitions(Blokk PRIVATE Blokk_Threading_Manual) # Define manual
    else()
        # The user entered an unrecognized threading type, throw an error
        message(FATAL_ERROR "BlokkForge: Unrecognized threading type passed (${THREADING_TYPE_UPPER}). Recognized types are: Manual and Adaptive")
    endif()
else()
    # Default to adaptive threading
    message("Threading system not set. Deafulting to: Adaptive")
    target_compile_definitions(Blokk PRIVATE Blokk_Threading_AdaptiveTiming)
endif()

# Camera enabled
string(JSON CAMERA_ENABLED ERROR JSON_ERR GET "${JSON_DATA}" "camera_enabled")
if("${JSON_ERR}" STREQUAL "NOTFOUND") # No error, the user has defined
    string(TOUPPER "${CAMERA_ENABLED}" CAMERA_ENABLED_UPPER) # Convert to uppercase
    if(CAMERA_ENABLED_UPPER) # User set to adaptive
        message("Camera enabled")
        target_compile_definitions(Blokk PRIVATE Blokk_CamEnabled) # Define camera enabled
    else()
        # The user entered something other than true
        message("Camera not enabled")
    endif()
else()
    # Default to adaptive threading
    message("Camera enabled not set. Deafulting to: false")
endif()
