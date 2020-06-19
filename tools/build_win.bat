@echo off
REM  "Usage ./build_win.bat [-hrcxqq]"
REM  "-h  Show this info"
REM  "-r  Build release (DEFAULT, optimizations, -O2)"
REM  "-c  Build clean (remove previous binarys / builds)"
REM  "-x  Don't build examples"
REM  "-m  Force use MinGW (GCC/G++)"
REM  "-l  Force use LLVM (clang/clang++)"
REM  "-q  Quiet"
REM  "-qq Really Quiet"

setlocal EnableDelayedExpansion
setlocal EnableExtensions

:ARG_LOOP
set ARG=%1
if "!ARG!" == "" ( goto BUILD_ASTERA )
IF NOT "x!ARG!" == "x!ARG:h=!" (
  goto HELP
)
IF NOT "x!ARG!" == "x!ARG:r=!" (
  set BUILD_RELEASE=1
)
IF NOT "x!ARG!" == "x!ARG:c=!" (
  goto CLEAN
)
IF NOT "x!ARG!" == "x!ARG:x=!" (
  set NO_EXAMPLES=1
)
IF NOT "x!ARG!" == "x!ARG:l=!" (
  set FORCE_LLVM=1
)
IF NOT "x!ARG!" == "x!ARG:m=!" (
  set FORCE_MINGW=1
)
IF NOT "x!ARG!" == "x!ARG:q=!" (
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
echo Usage ./build_win.bat [-hrcxqq]
echo -h  Show this info
echo -r  Build release (DEFAULT, optimizations, -O2)
echo -c  Build clean (remove previous binarys / builds)
echo -x  Don't build astera
echo -m  Force use MinGW (GCC/G++)
echo -l  Force use LLVM (clang/clang++)
echo -q  Quiet
echo -qq Really Quiet
exit /B

:BUILD_ASTERA

REM change directories to the root directory
cd ../

IF NOT DEFINED QUIET (
  echo BUILD INFO: Generating Makefiles
)

REM Generate Makefiles with CMake

IF DEFINED FORCE_LLVM (
  set C_COMPILER=clang
  set CXX_COMPILER=clang++
) ELSE IF DEFINED FORCE_MINGW (
  set C_COMPILER=gcc
  set CXX_COMPILER=g++
)

IF DEFINED NO_EXAMPLES (
  set EXAMPLES=OFF
) ELSE (
  set EXAMPLES=ON
)

IF DEFINED QUIET (
  IF DEFINED C_COMPILER (
    cmake -Bbuild -S. -DCMAKE_C_COMPILER=!C_COMPILER! -DCMAKE_CXX_COMPILER=!CXX_COMPILER! -DASTERA_BUILD_EXAMPLES=!EXAMPLES! > NUL 2&1
  ) ELSE (
    cmake -Bbuild -S. -DASTERA_BUILD_EXAMPLES=!EXAMPLES! > NUL 2&1   
  )
) ELSE (
  IF DEFINED C_COMPILER (
    cmake -Bbuild -S. -DCMAKE_C_COMPILER=!C_COMPILER! -DCMAKE_CXX_COMPILER=!CXX_COMPILER! -DASTERA_BUILD_EXAMPLES=!EXAMPLES!
  ) ELSE (
    cmake -Bbuild -S. -DASTERA_BUILD_EXAMPLES=!EXAMPLES!
  )
)

IF NOT DEFINED QUIET (
  echo BUILD INFO: Building libraries
)

REM BUild the libraries
IF DEFINED QUIET (
  cmake --build build > NUL 2&1
) ELSE (
  cmake --build build
)

IF NOT DEFINED QUIET (
  echo BUILD INFO: Complete
)

REM move back to the tools directory
cd tools/

exit /B

:CLEAN

REM Move to the root directory
cd ../

IF EXIST build (
  IF NOT DEFINED QUIET (
    echo CLEAN INFO: Previous build found, deleting it.
  )
  del /Q build
  rmdir /Q build
) ELSE (
  IF NOT DEFINED QUIET (
    echo CLEAN INFO: No previous builds found, exiting.
  )
  cd tools/ 
  exit /B
)

cd tools/
exit /B
