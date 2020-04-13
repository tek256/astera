cmake_minimum_required(VERSION 3.16)
project(astera VERSION 0.1.0 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON CACHE BOOL "Prevents cmake -S. -B.")
set(CMAKE_DISABLE_SOURCE_CHANGES ON CACHE BOOL "Prevent writing files to CMAKE_SOURCE_DIR under configure")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0075 NEW)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_FIND_FRAMEWORK FIRST)

include(CMakeDependentOption)

find_package(OpenGL REQUIRED)

cmake_dependent_option(ASTERA_BUILD_EXAMPLES 
  "Build astera's examples" ON
    "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" ON)

# TODO: put set(OPENAL_<option> values here)
add_subdirectory(${PROJECT_SOURCE_DIR}/dep/openal-soft)
add_subdirectory(${PROJECT_SOURCE_DIR}/dep/glfw)

set(ALSOFT_DLOPEN ON)
set(ALSOFT_WERROR OFF)
set(ALSOFT_UTILS OFF)
set(ALSOFT_EXAMPLES OFF)
set(ALSOFT_TESTS OFF)
set(ALSOFT_CONFIG OFF)
set(ALSOFT_HRTF_DEFS OFF)
set(ALSOFT_INSTALL OFF)

# set(GLFW_<option>)
set(BUILD_SHARED_LIBS ON)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_VULKAN_STATIC OFF)


if (NOT TARGET OpenAL::AL)
  add_library(OpenAL::AL ALIAS OpenAL)
endif()

file(GLOB sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${sources})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/dep/nanovg/nanovg.c)
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/dep/getopt/getopt.c)
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
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dep/getopt>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INCLUDE_INSTALLDIR}>)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<PLATFORM_ID:FreeBSD>:FreeBSD>
    $<$<PLATFORM_ID:OpenBSD>:OpenBSD>
    $<$<PLATFORM_ID:NetBSD>:NetBSD>
    $<$<PLATFORM_ID:Darwin>:OSX>)
target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)
target_link_libraries(${PROJECT_NAME}
  PUBLIC
    OpenGL::GL
    OpenAL::AL
    $<$<NOT:$<PLATFORM_ID:Windows>>:m>
    glfw)

if(ASTERA_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
