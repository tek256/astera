file(GLOB entries LIST_DIRECTORIES ON "${CMAKE_CURRENT_SOURCE_DIR}/*")

# Go thru and get each of the tools in their own directories
foreach (entry IN LISTS entries)
  if (IS_DIRECTORY "${entry}")
    list(APPEND directories "${entry}")
  endif()
endforeach()

# Go thru each of the directories and compile each
foreach(directory IN LISTS directories)
  file(GLOB sources CONFIGURE_DEPENDS "${directory}/*.c") 
  get_filename_component(name "${directory}" NAME)

  set(BUILD_SHARED_LIBS OFF)

  add_executable(${name})
  target_sources(${name} PRIVATE ${sources})
  target_link_libraries(${name} PRIVATE ${PROJECT_NAME})
endforeach()
