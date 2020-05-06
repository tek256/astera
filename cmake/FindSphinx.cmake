include(FindPackageHandleStandardArgs)

find_package(Python COMPONENTS Interpreter REQUIRED QUIET)
find_program(Sphinx_Build_EXECUTABLE NAMES sphinx-build)

if (Sphinx_Build_EXECUTABLE)
  set(Sphinx_Build_FOUND YES)
endif()

find_package_handle_standard_args(Sphinx
  REQUIRED_VARS Sphinx_Build_EXECUTABLE
  HANDLE_COMPONENTS)

if (Sphinx_Build_FOUND AND NOT TARGET Sphinx::Build)
  add_executable(Sphinx::Build IMPORTED)
  set_property(TARGET Sphinx::Build
    PROPERTY
      IMPORTED_LOCATION ${Sphinx_Build_EXECUTABLE})
endif()
