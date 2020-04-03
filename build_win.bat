@echo off

  REM  "Usage ./build_unix.sh [-hardiecs]"
  REM  "-h  Show this info"
  REM  "-a  Build all (deps, headers, examples)"
  REM  "-r  Build release (DEFAULT, optimizations, -O2)"
  REM  "-d  Build debug (-g, no optimmizations)"
  REM  "-i  Build include headers"
  REM  "-e  Build examples"
  REM  "-c  Build clean (remove previous binarys / builds)"
  REM  "-s  Build static library"
  REM  "-n  Don't rebuild dependencies"
  REM  "-x  Don't build astera"
  REM  "-q  Quiet"
  REM  "-qq Really Quiet"

setlocal EnableDelayedExpansion
setlocal EnableExtensions

:ARG_LOOP
set ARG=%1
if "!ARG!" == "" ( goto PREPARE )
IF NOT "x!ARG!" == "x!ARG:h=!" (
  goto HELP
)
IF NOT "x!ARG!" == "x!ARG:a=!" (
  set BUILD_ALL=1
):
IF NOT "x!ARG!" == "x!ARG:r=!" (
  set BUILD_RELEASE=1
)
IF NOT "x!ARG!" == "x!ARG:i=! (
  set BUILD_INCLUDES=1
)
IF NOT "x!ARG!" == "x!ARG:e=! (
  set BUILD_EXAMPLES=1
)
IF NOT "x!ARG!" == "x!ARG:c=!" (
  goto CLEAN
)
IF NOT "x!ARG!" == "x!ARG:s=!" (
  set BUILD_STATIC=1
)
IF NOT "x!ARG!" == "x!ARG:n=! (
  set NO_REBUILD=1
)
IF NOT "x!ARG!" == "x!ARG:x=! (
  set NO_ASTERA=1
)
IF NOT "x!ARG!" == "x!ARG:m=! (
  set FORCE_MINGW=1
)
IF NOT "x!ARG!" == "x!ARG:l=! (
  set FORCE_LLVM=1
)
IF NOT "x!ARG!" == "x!ARG:q=! (
  IF NOT DEFINED QUIET (
    set QUIET=1
  ) ELSE (
    set REALLY_QUIET=1
  )
)
IF NOT "%1" == "" (
  shift /1
  goto ARG_LOOP
)

:HELP
echo Usage build_windows.bat [-hardiecslmqq]
echo -h  Show this info
echo -a  Build all (deps, headers, examples)
echo -r  Build release (DEFAULT, optimizations, -O2)
echo -d  Build debug (-g, no optimmizations)
echo -i  Build include headers
echo -e  Build examples
echo -c  Build clean (remove previous binarys / builds)
echo -s  Build static library
echo -n  Don't rebuild dependencies
echo -x  Don't build astera
echo -m  Force using MinGW (GCC/G++)
echo -l  Force using LLVM/Clang
echo -q  Suppress output from the script
echo -qq Suppress all output from the script
exit /B

:PREPARE
REM Define what compilation environment we'll be using
IF DEFINED FORCE_MINGW (
  set GENERATOR="MinGW Makefiles"
  set C_COMPILER="gcc"
  set CXX_COMPILER="g++"
  ) ELSE (
    IF DEFINED FORCE_LLVM (
      set GENERATOR=""
      set C_COMPILER="clang"
      set CXX_COMPILER="clang++"
    ) ELSE (
      set GENERATOR=""
      set C_COMPILER="cl"
      set CXX_COMPILER="cl"
    )
  )
)

REM Build all the include headers needed to compile with astera
:BUILD_INCLUDES
IF NOT EXIST !ROOT_DIR!\include\ (
  mkdir !ROOT_DIR!\include\
)

REM Copy OpenAL-Soft's headers
IF NOT EXIST !ROOT_DIR!\include\AL\ (
  xcopy /s dep\openal-soft\include include
)
REM GLFW's headers
IF NOT EXIST !ROOT_DIR!\include\GLFW\ (
  xcopy /s dep\glfw\include include
)

REM Copy Astera's headers
IF NOT EXIST !ROOT_DIR!\include\astera\ (
  mkdir !ROOT_DIR!\include\astera\
  xcopy /s src\*.h include\astera
)

:BUILD_GLFW
IF EXIST !ROOT_DIR!\dep\glfw\build (
  IF NOT DEFINED NO_REBUILD (
      del /Q !ROOT_DIR!\dep\glfw\build
      rmdir !ROOT_DIR!\dep\glfw\build
  )
)

IF NOT EXIST !ROOT_DIR!\dep\glfw\build (
  mkdir !ROOT_DIR!\dep\glfw\build
)



:BUILD_ASTERA
IF NOT DEFINED NO_ASTERA (
  IF NOT DEFINED QUIET (
    echo "BUILD INFO: Generating Makefiles"
  )

  REM Generate Makefiles with CMake
  IF DEFINED QUIET (
    IF DEFINED FORCE_LLVM (
      cmake -Bbuild -S. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ > NUL 2&1
    ) ELSE (
      IF DEFINED FORCE_MINGW (
        cmake -Bbuild -S. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ > NUL 2&1
      ) ELSE (
        cmake -Bbuild -S. > NUL 2&1
      )
    )
  ) ELSE (
    IF DEFINED FORCE_LLVM (
      cmake -Bbuild -S. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
    ) ELSE (
      IF DEFINED FORCE_MINGW (
        cmake -Bbuild -S. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
      ) ELSE (
        cmake -Bbuild -S.
      )
    )
  )

  IF NOT DEFINED QUIET (
    echo "BUILD INFO: Building libraries"
  )

  REM BUild the libraries
  IF DEFINED QUIET (
    cmake --build build > NUL 2&1
  ) ELSE (
    cmake --build build
  )
)

:BUILD_EXAMPLES
REM Build the examples
IF DEFINED BUILD_EXAMPLES (
  REM TODO Build examples
)
