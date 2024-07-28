cmake_minimum_required(VERSION 3.16)
project(astera VERSION 0.0.1 LANGUAGES C)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON CACHE BOOL "Prevents cmake -S. -B.")
set(CMAKE_DISABLE_SOURCE_CHANGES ON CACHE BOOL "Prevent writing files to CMAKE_SOURCE_DIR under configure")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_FIND_FRAMEWORK LAST)

include(CMakeDependentOption)
include(GenerateExportHeader)
include(FetchContent)

FetchContent_Declare(openal-windows
  DOWNLOAD_EXTRACT_TIMESTAMP YES
  URL "https://openal-soft.org/openal-binaries/openal-soft-1.20.1-bin.zip")
if (WIN32)
  FetchContent_MakeAvailable(openal-windows)
  set(ENV{OPENALDIR} "${openal-windows_SOURCE_DIR}")
endif()

find_package(OpenAL REQUIRED)

if (OPENAL_FOUND AND NOT TARGET OpenAL::OpenAL)
  add_library(OpenAL::OpenAL UNKNOWN IMPORTED)
  set_target_properties(OpenAL::OpenAL
    PROPERTIES
      IMPORTED_LOCATION "${OPENAL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}")
endif()

find_package(OpenGL REQUIRED)

# If to build the `examples/` folder
cmake_dependent_option(ASTERA_BUILD_EXAMPLES 
  "Build astera's examples" ON
  "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" ON)

# Build out the utility tools
cmake_dependent_option(ASTERA_BUILD_TOOLS 
  "Build astera's tools" ON
  "CMAKE_BUILD_TYPE STREQUAL Debug" ON)

# Enables output using the ASTERA_DBG macro
cmake_dependent_option(ASTERA_DEBUG_OUTPUT
  "Enable Astera's internal debug output" ON 
  "CMAKE_BUILD_TYPE STREQUAL Debug" ON)

# Enables hardened compiler flags for astera 
cmake_dependent_option(ASTERA_HARDEN_ENGINE
  "Enable hardening compiler flags for astera " OFF
  "CMAKE_BUILD_TYPE STREQUAL Release" ON)

# Enable the inclusion of pak writing in the library
cmake_dependent_option(ASTERA_PAK_WRITE
  "Include pak writing functions in asset.c/h" ON
  "CMAKE_BUILD_TYPE STREQUAL Debug;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" ON)

# Enables ASAN & Pedantic output
option(ASTERA_DEBUG_ENGINE OFF)

# Disable OpenAL Audio Effects from being compiled in
# NOTE: Mac OSX OpenAL Effects are currently disabled by default
option(ASTERA_DISABLE_AUDIO_FX OFF)

# Set the GLFW Flags
set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_VULKAN_STATIC OFF)

# Add GLFW
add_subdirectory(${PROJECT_SOURCE_DIR}/dep/glfw EXCLUDE_FROM_ALL)

# Get all of our source files into a list
file(GLOB sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
#add_library(${PROJECT_NAME} STATIC)
add_library(${PROJECT_NAME} STATIC)

# Add our source files & nanovg
target_sources(${PROJECT_NAME} PRIVATE ${sources})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/dep/nanovg/nanovg.c)

if(APPLE)
  set(ASTERA_DISABLE_AUDIO_FX ON)
endif()

# Include headers
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/glad/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/stb>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INCLUDE_INSTALLDIR}>)

# Add all the preprocessor definitions for headers
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<BOOL:${ASTERA_DEBUG_ENGINE}>:ASTERA_DEBUG_OUTPUT>
    $<$<BOOL:${ASTERA_DEBUG_OUTPUT}>:ASTERA_DEBUG_OUTPUT>
    $<$<BOOL:${ASTERA_HARDEN_ENGINE}>:ASTERA_HARDEN_ENGINE>
    $<$<PLATFORM_ID:FreeBSD>:FreeBSD>
    $<$<PLATFORM_ID:OpenBSD>:OpenBSD>
    $<$<PLATFORM_ID:NetBSD>:NetBSD>
    $<$<PLATFORM_ID:Darwin>:OSX>
  PUBLIC
    $<$<BOOL:${ASTERA_DISABLE_AUDIO_FX}>:ASTERA_AL_NO_FX>
    $<$<BOOL:${ASTERA_PAK_WRITE}>:ASTERA_PAK_WRITE>)

# Set the C Standard to C99
target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

# Add warning output for the different compilers
target_compile_options(${PROJECT_NAME}
  PRIVATE
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
  $<$<C_COMPILER_ID:MSVC>:/W4>)

# Enables ASAN & Pedantics
if(ASTERA_DEBUG_ENGINE)
  target_compile_options(${PROJECT_NAME}
    PUBLIC
    $<$<C_COMPILER_ID:MSVC>:/fsanitize=address>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>: -g -fsanitize=address -fno-omit-frame-pointer -O2 -g>)
  
  target_link_options(${PROJECT_NAME}
    PUBLIC
    $<$<C_COMPILER_ID:MSVC>:/fsanitize=address>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-g -fsanitize=address>)
endif()

if(ASTERA_HARDEN_ENGINE)
  target_compile_options(${PROJECT_NAME} 
      PUBLIC
      $<$<NOT:$<C_COMPILER_ID:MSVC>>: -O2 -Wall -Wformat=2 -Wconversion -Wtrampolines -Wimplicit-fallthrough -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS -fstack-protector-strong -Wl,-z,nodlopen -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now>)
endif()

# Link against the various libraries we need
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    OpenGL::GL
    $<$<NOT:$<PLATFORM_ID:Windows>>:m>
    glfw
    $<$<PLATFORM_ID:Windows>:XInput>
    OpenAL::OpenAL)

set_property(TARGET astera PROPERTY C_VISIBILITY_PRESET "hidden")
set_property(TARGET astera PROPERTY VERSION ${PROJECT_VERSION})

install(TARGETS ${PROJECT_NAME} glfw
  EXPORT ${PROJECT_NAME}-targets
  INCLUDES DESTINATION "${CMAKE_INCLUDE_INSTALLDIR}")
install(EXPORT ${PROJECT_NAME}-targets
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  NAMESPACE ${PROJECT_NAME})
install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/"
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h")

if(ASTERA_BUILD_EXAMPLES)
  if(EXISTS "${PROJECT_SOURCE_DIR}/examples")
    add_subdirectory("${PROJECT_SOURCE_DIR}/examples")
  else()
    message(WARNING "Unable to find examples directory, disabling ASTERA_BUILD_EXAMPLES")
    set(ASTERA_BUILD_EXAMPLES OFF)
  endif()
endif()

if(ASTERA_BUILD_TOOLS)
  if(EXISTS "${PROJECT_SOURCE_DIR}/tools")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tools")
  else()
    message(WARNING "Unable to find tools directory, disabling ASTERA_BUILD_TOOLS")
    set(ASTERA_BUILD_TOOLS OFF)
  endif()
endif()
