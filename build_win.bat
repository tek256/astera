@echo off

set SOURCES=src\*.c
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
)
IF NOT "x!ARG!" == "x!ARG:r=!" (
  set BUILD_RELEASE=1
)
IF NOT "x!ARG!" == "x!ARG:d=!" (
  set BUILD_DEBUG=1
)
IF NOT "x!ARG!" == "x!ARG:c=!" (
  goto CLEAN
)
IF NOT "x!ARG!" == "x!ARG:s=!" (
  set BUILD_STATIC=1
)
IF NOT "%1" == "" (
  shift /1
  goto ARG_LOOP
)

:HELP
echo Usage windows_build.bat [-hardcs]
echo -h Show this info
echo -a Build all (including deps)
echo -r Build release (DEFAULT, optimizations, -O2)
echo -d Build debug (-g, no optimmizations)
echo -c Build clean (remove previous binarys / builds)
echo -s Build static library
exit /B

:PREPARE
IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat"
) ELSE (
  REM Initialize your vc environment here if the defaults don't work
  REM  set VC_INIT="C:\your\path\here\vcvarsall.bat"
  REM And then remove/comment out the following two lines
  echo "Couldn't find vcvarsall.bat or vcbuildtools.bat, please set it manually."
  exit /B
)

:BUILD
set "ROOT_DIR=%CD%"
set "SOURCES=%!ROOT_DIR!\!SOURCES!"
set CC_FLAGS=/01 /GL
set WARNING_FLAGS=
set SUBSYSTEM_FLAGS=/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup
set LD_FLAGS=/link /LTCG kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
set OUTPUT_DIR="!ROOT_DIR!"
IF DEFINED BUILD_DEBUG (
  set CC_FLAGS=/Od /Zi
  set WARNING_FLAGS=/Wall
  set SUBSYSTEM_FLAGS=
  set LD_FLAGS=/link /LTCG kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
  set OUTPUT_DIR="!ROOT_DIR!"
)
