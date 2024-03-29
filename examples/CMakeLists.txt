if(APPLE)
  message(WARNING "OpenGL is deprecated on macOs 10.14 (Mojave)!")
endif()

string(CONCAT opengl-deprecation $<AND:
  $<PLATFORM_ID:Darwin>,
  $<VERSION_LESS:${CMAKE_SYSTEM_VERSION},18>
>)

file(GLOB entries LIST_FILES ON "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_custom_target(${PROJECT_NAME}-examples-resources ALL
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/resources"
      "${CMAKE_CURRENT_BINARY_DIR}/resources"
  COMMENT "Copying resources directory"
  VERBATIM)

find_package(OpenAL REQUIRED)

foreach(example IN LISTS entries)
  get_filename_component(name "${example}" NAME_WLE)

  set(BUILD_SHARED_LIBS OFF)
  
  add_executable(${name})
  target_sources(${name} PRIVATE ${example})
  target_compile_definitions(${name} 
    PRIVATE
    $<${opengl-deprecation}:GL_SILENCE_DEPRECATION>)

  target_link_libraries(${name} 
        PRIVATE 
          ${PROJECT_NAME})

          endforeach()
