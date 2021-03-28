cmake_minimum_required(VERSION 3.16)
project(astera VERSION 0.1.0 LANGUAGES C)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON CACHE BOOL "Prevents cmake -S. -B.")
set(CMAKE_DISABLE_SOURCE_CHANGES ON CACHE BOOL "Prevent writing files to CMAKE_SOURCE_DIR under configure")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_FIND_FRAMEWORK FIRST)

include(CMakeDependentOption)
include(GenerateExportHeader)
include(FetchContent)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/dep/cmake")

# Use OpenAL-Soft's binaries for Windows
if (WIN32)
  FetchContent_Declare(openalwin
    URL "https://openal-soft.org/openal-binaries/openal-soft-1.20.1-bin.zip")
  if (NOT openalwin_POPULATED)
    FetchContent_Populate(openalwin)
  endif()
  list(APPEND CMAKE_PREFIX_PATH "${openalwin_SOURCE_DIR}")
endif()

find_package(OpenGL REQUIRED)

# OpenAL must be installed on the system, env "OPENALDIR" must be set
find_package(OpenALSoft)
find_package(OpenAL)

if(OPENALSOFT_FOUND)
  if(OPENALSOFT_FOUND AND NOT TARGET OpenAL::AL)
    add_library(OpenAL::AL UNKNOWN IMPORTED)
    set_target_properties(OpenAL::AL PROPERTIES
      IMPORTED_LOCATION "${OPENALSOFT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENALSOFT_INCLUDE_DIR}")
   endif()
elseif(OPENAL_FOUND)
  message("ALSOFT FOUND ${OPENAL-SOFTFOUND}")
  if(OPENAL_FOUND AND NOT TARGET OpenAL::AL)
    add_library(OpenAL::AL UNKNOWN IMPORTED)
    set_target_properties(OpenAL::AL PROPERTIES
      IMPORTED_LOCATION "${OPENAL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENAL_INCLUDE_DIR}")
  endif()
else()
  message(FATAL_ERROR "Could not find OpenAL or OpenAL-Soft")
endif()

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

# Enable the inclusion of pak writing in the library
cmake_dependent_option(ASTERA_PAK_WRITE
  "Include pak writing functions in asset.c/h" ON
  "CMAKE_BUILD_TYPE STREQUAL Debug" ON)

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

file(GLOB sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE ${sources})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/dep/nanovg/nanovg.c)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/glad/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep>
    $<$<NOT:$<PLATFORM_ID:Darwin>>:$<BUILD_INTERFACE:${OPENAL_INCLUDE_DIR}>>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/stb>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INCLUDE_INSTALLDIR}>)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<BOOL:${ASTERA_DEBUG_ENGINE}>:ASTERA_DEBUG_OUTPUT>
    $<$<BOOL:${ASTERA_DEBUG_OUTPUT}>:ASTERA_DEBUG_OUTPUT>
    $<$<PLATFORM_ID:FreeBSD>:FreeBSD>
    $<$<PLATFORM_ID:OpenBSD>:OpenBSD>
    $<$<PLATFORM_ID:NetBSD>:NetBSD>
    $<$<PLATFORM_ID:Darwin>:OSX>
  PUBLIC
    $<$<BOOL:${ASTERA_DISABLE_AUDIO_FX}>:ASTERA_AL_NO_FX>
    $<$<BOOL:${ASTERA_PAK_WRITE}>:ASTERA_PAK_WRITE>)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

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

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    OpenGL::GL
    $<$<NOT:$<PLATFORM_ID:Windows>>:m>
    glfw
  PRIVATE
    OpenAL::AL)


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
  if(EXISTS examples)
    add_subdirectory(examples)
  else()
    message(WARNING "Unable to find exampels directory, disabling ASTERA_BUILD_EXAMPLES")
    set(ASTERA_BUILD_EXAMPLES OFF)
  endif()
endif()

if(ASTERA_BUILD_TOOLS)
  if(EXISTS tools)
    add_subdirectory(tools)
  else()
    message(WARNING "Unable to find tools directory, disabling ASTERA_BUILD_TOOLS")
    set(ASTERA_BUILD_TOOLS OFF)
  endif()
endif()
