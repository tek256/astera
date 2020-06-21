cmake_minimum_required(VERSION 3.16)
project(astera VERSION 0.1.0 LANGUAGES C)

# list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON CACHE BOOL "Prevents cmake -S. -B.")
set(CMAKE_DISABLE_SOURCE_CHANGES ON CACHE BOOL "Prevent writing files to CMAKE_SOURCE_DIR under configure")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_FIND_FRAMEWORK FIRST)

include(CMakeDependentOption)
include(GenerateExportHeader)

find_package(OpenGL REQUIRED)

# If to build the `examples/` folder
cmake_dependent_option(ASTERA_BUILD_EXAMPLES 
  "Build astera's examples" ON
  "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" ON)

# Enables output using the ASTERA_DBG macro
option(ASTERA_DEBUG_OUTPUT "Enable Astera's internal debug output" ON)

# Enables ASAN & Pedantic output
option(ASTERA_DEBUG_ENGINE
  "Enable debug options for astera's compilation" OFF)

set(ALSOFT_DLOPEN ON)
set(ALSOFT_WERROR OFF)
set(ALSOFT_UTILS OFF)
set(ALSOFT_EXAMPLES OFF)
set(ALSOFT_TESTS OFF)
set(ALSOFT_CONFIG OFF)
set(ALSOFT_HRTF_DEFS OFF)
set(ALSOFT_INSTALL OFF)

set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_VULKAN_STATIC OFF)

set(LIBTYPE STATIC)
add_subdirectory(${PROJECT_SOURCE_DIR}/dep/openal-soft EXCLUDE_FROM_ALL)
set_property(TARGET OpenAL PROPERTY POSITION_INDEPENDENT_CODE ON)
unset(LIBTYPE)
add_subdirectory(${PROJECT_SOURCE_DIR}/dep/glfw EXCLUDE_FROM_ALL)

if (NOT TARGET OpenAL::AL)
  add_library(OpenAL::AL ALIAS OpenAL)
endif()

file(GLOB sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PRIVATE ${sources})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/dep/nanovg/nanovg.c)
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/dep/zip/zip.c)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/openal-soft/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/glad/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/openal-soft>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/stb>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/zip>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INCLUDE_INSTALLDIR}>)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<BOOL:${ASTERA_DEBUG_ENGINE}>:ASTERA_DEBUG_OUTPUT>
    $<$<PLATFORM_ID:FreeBSD>:FreeBSD>
    $<$<PLATFORM_ID:OpenBSD>:OpenBSD>
    $<$<PLATFORM_ID:NetBSD>:NetBSD>
    $<$<PLATFORM_ID:Darwin>:OSX>)
target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

# Enables ASAN & Pedantics
if(ASTERA_DEBUG_ENGINE)
  target_compile_options(${PROJECT_NAME}
    PRIVATE
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
    $<$<C_COMPILER_ID:MSVC>:/WX /W4>
    PUBLIC
    $<$<C_COMPILER_ID:MSVC>:/fsanitize=address>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>: -fsanitize=address -fno-omit-frame-pointer -O2 -g>)# -fstack-protector-all -fstack-protector-strong -D_GLIBCXX_ASSERTIONS -fasynchronous-unwind-tables -D_FORTIFY_SOURCE=2>)
    target_link_options(${PROJECT_NAME}
       PUBLIC
       $<$<C_COMPILER_ID:MSVC>:/fsanitize=address>
       $<$<NOT:$<C_COMPILER_ID:MSVC>>:-fsanitize=address>)
else()
  target_compile_options(${PROJECT_NAME} PUBLIC -w)
endif()

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    OpenGL::GL
    OpenAL::AL
    $<$<NOT:$<PLATFORM_ID:Windows>>:m>
    glfw)

set_property(TARGET astera PROPERTY C_VISIBILITY_PRESET "hidden")
set_property(TARGET astera PROPERTY VERSION ${PROJECT_VERSION})

install(TARGETS astera OpenAL glfw
  EXPORT astera-targets
  INCLUDES DESTINATION "${CMAKE_INCLUDE_INSTALLDIR}")
install(EXPORT astera-targets
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  NAMESPACE astera)
install(DIRECTORY include/ "${PROJECT_BINARY_DIR}/include/"
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h")

if(ASTERA_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

