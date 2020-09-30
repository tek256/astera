#!/bin/sh

# Stop if something fails
set -e

# Usage: ./build_unix.sh [-hrcxqq]
#  -h  Show this info
#  -r  Build release (optimizations, -O2)
#  -c  Build clean (remove previous binarys / builds)
#  -x  Don't build examples
#  -q  Quiet output
#  -qq Really quiet output

build_cleanup() {
  current_dir=$PWD
  cd "$1"

  if [ -n "$QUIET" ] && [ -z "$REALLY_QUIET" ]; then
    echo "CLEANUP: Starting"
  fi

  # Remove any old builds of astera under `build`
  if [ -d "build" ]; then
    if [ -z "$QUIET" ]; then
      echo "CLEANUP: Removing old build."
    fi
    rm -rf build
  fi

  if [ -n "$QUIET" ] && [ -z "$REALLY_QUIET" ]; then
    echo "CLEANUP: Complete"
  fi

  # Move back to the previous location
  cd "$current_dir"
}

# Parse thru all the options
while getopts ":hrcxqq" opt; do
  case $opt in
    h)
      echo "Usage ./build_unix.sh [-hardiecs]"
      echo "-h  Show this info"
      echo "-r  Build release (optimizations, -O2)"
      echo "-c  Build clean (remove previous build)"
      echo "-x  Don't build examples"
      echo "-q  Quiet"
      echo "-qq Really Quiet"
      exit 0
      ;;
    r)
      BUILD_RELEASE="1"
      ;;
    c)
      BUILD_CLEAN="1"
      ;;
    x)
      NO_EXAMPLES="1"
      ;;
    q)
      if [ -n "$QUIET" ]; then
        REALLY_QUIET="1"
      else 
        QUIET="1"
      fi
      ;;
    \?)
      echo "Invalid option passed: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

# Starting directory
STARTING_DIR="${PWD}"

# Back out of the tools/ directory to the root directory
cd ../

# Get the directory above tools
ROOT_DIR="${PWD}"

# Check if the clean flag was passed
if [ -n "$BUILD_CLEAN" ]; then
  if [ -z "$REALLY_QUIET" ]; then
    echo "BUILD INFO: Clean selected."
  fi

  build_cleanup "$ROOT_DIR"

  if [ -z "$REALLY_QUIET" ]; then
    echo "BUILD INFO: Cleanup complete."
  fi

  exit 0
fi

# Set the build type
if [ -z "$BUILD_RELEASE" ]; then
  BUILD_TYPE="Debug"
else
  BUILD_TYPE="Release"
fi

# Set if we should build examples
if [ -z "$NO_EXAMPLES" ]; then
  BUILD_EXAMPLES="ON"
else
  BUILD_EXAMPLES="OFF"
fi

if [ -z "$REALLY_QUIET" ]; then
  echo "BUILD INFO: Generate CMake Build"
fi

if [ -z "$QUIET" ]; then
    cmake -S. -Bbuild -DASTERA_BUILD_EXAMPLES="$BUILD_EXAMPLES" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
else 
    cmake -S. -Bbuild -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DASTERA_BUILD_EXAMPLES="$BUILD_EXAMPLES" >> /dev/null 2>&1
fi

# BUild the actual project
if [ -z "$REALLY_QUIET" ]; then
  echo "BUILD INFO: Building astera"
fi

if [ -z "$QUIET" ]; then
  cmake --build build
else
  cmake --build build >> /dev/null 2>&1
fi

if [ -z "$REALLY_QUET" ]; then
  echo "BUILD INFO: Build complete"
fi

cd "$STARTING_DIR"

# Exit the script successfully
exit 0
