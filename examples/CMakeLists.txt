if(APPLE)
  message(WARNING "OpenGL is deprecated on macOs 10.14 (Mojave)!")
endif()

string(CONCAT opengl-deprecation $<AND:
  $<PLATFORM_ID:Darwin>,
  $<VERSION_LESS:${CMAKE_SYSTEM_VERSION},18>
>)

file(GLOB entries LIST_DIRECTORIES ON "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach (entry IN LISTS entries)
  if (IS_DIRECTORY "${entry}")
    list(APPEND directories "${entry}")
  endif()
endforeach()
list(REMOVE_ITEM directories "${CMAKE_CURRENT_SOURCE_DIR}/resources")

add_custom_target(${PROJECT_NAME}-examples-resources ALL
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/resources"
      "${CMAKE_CURRENT_BINARY_DIR}/resources"
      #MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/resources"
  COMMENT "Copying resources directory"
  VERBATIM)

foreach(directory IN LISTS directories)
  file(GLOB sources CONFIGURE_DEPENDS "${directory}/*.c") 
  get_filename_component(name "${directory}" NAME)

  set(BUILD_SHARED_LIBS OFF)

  add_executable(${name})
  target_sources(${name} PRIVATE ${sources})
  target_compile_definitions(${name} 
    PRIVATE
    $<${opengl-deprecation}:GL_SILENCE_DEPRECATION>)
  target_link_libraries(${name} PRIVATE ${PROJECT_NAME})
endforeach()
