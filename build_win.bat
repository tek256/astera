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
IF NOT "x!ARG!" == "x!ARG:d=!" (
  set BUILD_DEBUG=1
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
echo -m  Force using MinGW (GCC)
echo -l  Force using LLVM/Clang
echo -q  Suppress output from the script
echo -qq Suppress all output from the script
exit /B

:PREPARE
REM Define what compilation environment we'll be using
IF DEFINED FORCE_MINGW (
  IF EXIST "C:\MinGW\" (
    set MINGW_SET=1
    set USE_GCC=1
  ) ELSE IF EXIST "C:\msys\mingw32" (
    set MSYS_SET=1
    set USE_GCC=1
  ) ELSE IF EXIST "C:\msys\mingw64" (
    set MSYS_SET=1
    set USE_GCC=1
  ) ELSE IF EXIST "C:\msys64\mingw32" (
    set MSYS_SET=1
    set USE_GCC=1
  ) ELSE IF EXIST "C:\msys64\mingw64" (
    set MSYS_SET=1
    set USE_GCC=1
  ) ELSE (
    REM Unable to find MSYS or MinGW
    echo "Unable to find MinGW or MSYS in order to compile with GCC."
    exit /B
  )
) ELSE (
  IF DEFINED FORCE_LLVM (
    IF EXIST "C:\Program Files (x86)\LLVM\" (
      set USE_LLVM=1
    ) ELSE IF EXIST "C:\Program Files\LLVM\" (
      set USE_LLVM=1
    ) ELSE (
      REM No LLVM Installation found
      echo "Couldn't find LLVM install, please make sure it's set"
      exit /B
    )
  ) ELSE (
    IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    set VC_INIT="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
    set USE_VC=1
    ) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
      set VC_INIT="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
      set USE_VC=1
    ) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat" (
      set VC_INIT="C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat"
      set USE_VC=1
    ) ELSE (
      REM Initialize your vc environment here if the defaults don't work
      REM set VC_INIT="C:\your\path\here\vcvarsall.bat"
      REM And then remove/comment out the following two lines
      echo "Couldn't find vcvarsall.bat or vcbuildtools.bat, please set it manually."
      exit /B
    )
  )
)

IF DEFINED USE_VC (
  IF DEFINED VC17 (

  ) ELSE (

  )
) ELSE (
  IF DEFINED USE_MINGW ( 
    IF NOT DEFINED QUIET echo "Building using MinGW"
  ) ELSE IF DEFINED USE_LLVM (
    IF NOT DEFINED QUIET echo "Building using LLVM/Clang"
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

cd !ROOT_DIR!\dep\glfw\build

IF DEFINED USE_VC (
  REM IF DEFINED QUIET (
  REM   cmake ..\ -G "Visual Studio 15 2017" > NUL 2&1
  REM ) ELSE (
  REM   cmake ..\ -G "Visual Studio 15 2017"
  REM )
) ELSE (
  IF DEFINED USE_GCC (
    IF DEFINED QUIET (
      IF NOT DEFINED REALLY_QUIET (
        echo GLFW BUILD: Generating GLFW Makefile
      )

      cmake ..\ -G "MinGW Makefiles" > NUL 2&1

      IF NOT DEFINED REALLY_QUIET (
        echo GLFW BUILD: Compiling GLFW
      )
      
      mingw32-make > NUL 2&1
    ) ELSE (
      echo GLFW BUILD: Generating GLFW Makefile
      cmake ..\ -G "MinGW Makefiles"

      echo GLFW BUILD: Compiling GLFW
      mingw32-make
    )
  ) ELSE (
    echo LOL
  )
)

cd !ROOT_DIR!
xcopy 

:BUILD_ASTERA
IF NOT DEFINED NO_ASTERA (
  set "ROOT_DIR=%CD%"
  set "SOURCES=%!ROOT_DIR!\!SOURCES!"

  REM Setup Compiler flags
  IF DEFINED USE_VC (
    REM set LD_FLAGS=/link /LTCG kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
    REM set OUTPUT_DIR="!ROOT_DIR!"

    REM IF DEFINED BUILD_DEBUG (
    REM   set CC_FLAGS=/Od /Zi
    REM   set WARNING_FLAGS=/Wall
    REM   set SUBSYSTEM_FLAGS=
    REM   IF NOT DEFINED QUIET echo Compiling in debug mode using msvc, flags: !CC_FLAGS! !WARNING_FLAGS! 
    REM ) ELSE (
    REM   set CC_FLAGS=/01 /GL
    REM   set WARNING_FLAGS=
    REM   set SUBSYSTEM_FLAGS=/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup
    REM   IF NOT DEFINED QUIET echo Compiling in release mode using msvc, flags: !CC_FLAGS! /link /LTCG
    REM )
  ) ELSE (
    IF DEFINED BUILD_DEBUG (
      set CC_FLAGS=-w -g
    ) ELSE IF DEFINED BUILD_RELEASE (
      set CC_FLAGS=-w -O2
    ) ELSE (
      set CC_FLAGS=-w
    )
  )

  REM IF NOT DEFINED VERBOSE ( 
  REM   set VERBOSITY_FLAG=/nologo
  REM )

  set "TMP_DIR=lib"

  REM Clear out old build
  IF EXIST !TMP_DIR!\ (
    IF NOT DEFINED NO_REBUILD (
      IF NOT DEFINED QUIET echo "Found cached astera, rebuilding"
      del /Q !TMP_DIR!
      rmdir !TMP_DIR!
    )
  )

  REM Create the temporary directory if it doesn't exist
  IF NOT EXIST !TMP_DIR!\ (
    mkdir !TMP_DIR!
  )

  cd !TMP_DIR!
  set C_FILES="!SRC!\main.c" "!SRC!\game.c" "!SRC!\render.c" "!SRC!\audio.c" "!SRC!\asset.c" "!SRC!\conf.c" "!SRC!\debug.c" "!SRC!\input.c" "!SRC!\sys.c" "!SRC!\ui.c"

  IF DEFINED USE_VC (
    set INCLUDES=/I"!ROOT_DIR!\dep\glfw\include" /I"!ROOT_DIR!\dep\openal-soft\include" /I"!ROOT_DIR!\dep\"

    IF DEFINED REALLY_QUIET (
      cl.exe /w /c !INCLUDES! !CC_FLAGS! !C_FILES! > NUL 2>&1 || exit /B
    ) ELSE (
      cl.exe /w /c !VERBOSITY_FLAG! !INCLUDES! !CC_FLAGS! !C_FILES! || exit /B
    )
  ) ELSE (
    set INCLUDES=-I"!ROOT_DIR!\dep\glfw\include" -I"!ROOT_DIR!\dep\openal-soft\include" -I"!ROOT_DIR!\dep\"
    IF DEFINED USE_GCC ( 
      IF DEFINED REALLY_QUIET (
        gcc !CC_FLAGS! !INCLUDES! !LIBRARIES! !C_FILES! -o astera > NUL 2&1 || exit /B
      ) ELSE (
        gcc !CC_FLAGS! !INCLUDES! !LIBRARIES! !C_FILES! -o astera || exit /B
      )
    ) ELSE IF DEFINED USE_LLVM (
      IF DEFINED REALLY_QUIET (
        clang !CC_FLAGS! !INCLUDES! !LIBRARIES! !C_FILES! -o astera > NUL 2>&1 || exit /B
      ) ELSE (
        clang !CC_FLAGS! !INCLUDES! !LIBRARIES! !C_FILES! -o astera || exit /B
      )
    )
  )

  IF NOT DEFINED QUIET echo Astera compiled into: !TMP_DIR!\
  cd !ROOT_DIR!
)

:BUILD_EXAMPLES
REM Build the examples
IF DEFINED BUILD_EXAMPLES (
  REM TODO Build examples
)