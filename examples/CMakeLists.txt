# TODO Everything, and consult izzy

project(astera_examples)

set(example_dirs animations audio basic_game config input music render_stress shaders sprites ui)
set(example_sources)
set(example_resources)

foreach(example_dir ${example_dirs})
  file(GLOB sources ${example_dir}/*.c)
  list(APPEND example_sources ${sources})

  file(GLOB resource ${example_dir}/resources/*)
  file(APPEN example_resources ${resources})
endforeach()

if (APPLE AND NOT CMAKE_SYSTEM STRLESS "Darwin-18.0.0")
  add_definitions(-DGL_SILENCE_DEPRECATION)
  MESSAGE(AUTHOR_WARNING "OpenGL is deprecated starting with macOS 10.14 (Mojave)!")
endif()


