# TODO Everything, and consult izzy

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

add_library(${PROJECT_NAME}-examples INTERFACE)
#add_custom_command(TARGET ${PROJECT_NAME}-examples POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:${PROJECT_NAME}-examples>/resources")
add_custom_target(${PROJECT_NAME}-examples-resources
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resources" "$<TARGET_FILE_DIR:${PROJECT_NAME}-examples>/resources"
  COMMENT "Copying examples resources"
  VERBATIM)
add_dependencies(${PROJECT_NAME}-examples ${PROJECT_NAME}-examples-resources)

foreach(directory IN LISTS directories)
  file(GLOB sources CONFIGURE_DEPENDS "${directory}/*.c") 
  get_filename_component(name "${directory}" NAME)
  add_executable(${name} WIN32)
  target_sources(${name} PRIVATE ${sources})
  target_compile_definitions(${name} 
    PRIVATE
    $<${opengl-deprecation}:GL_SILENCE_DEPRECATION>)

  target_link_libraries(${name} PRIVATE ${PROJECT_NAME}-examples)
  set_property(TARGET ${name} PROPERTY BUILD_RPATH $<TARGET_FILE_DIR:astera>)
endforeach()
