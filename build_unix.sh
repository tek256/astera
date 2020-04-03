#!/bin/sh

# NOTE: This is currently under development, for 0.01 it'll be changed to
# have library only, but for now we'll work with th executable

# Usage: ./build_unix.sh [-hardiecs]
#  -h  Show this info
#  -a  Build all (deps, headers, examples)
#  -r  Build release (DEFAULT, optimizations, -O2)
#  -d  Build debug (-g, no optimmizations)
#  -i  Build include headers
#  -e  Build examples
#  -c  Build clean (remove previous binarys / builds)
#  -x  Don't build astera
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

  # Clean up the include directories
  if [ -d "include" ]; then
    # If the astera headers exist, then remove them
    if [ -d "include/astera" ]; then
      if [ -z "$QUIET" ]; then
        echo "CLEANUP: Removing astera headers"
      fi
      rm -rf include/astera
    fi
    
    # If the GLFW headers exist, then remove them
    if [ -d "include/GLFW" ]; then
       if [ -z "$QUIET" ]; then
         echo "CLEANUP: Removing GLFW headers"
      fi
      rm -rf include/GLFW
    fi

    # If the OpenAL-Soft headers exist, then remove them
    if [ -d "include/AL" ]; then
       if [ -z "$QUIET" ]; then
         echo "CLEANUP: Removing OpenAL-Soft headers"
      fi
      rm -rf include/AL
    fi

    # Check if the include folder is empty, if so, remove it.
    if [ -z "$(ls -A include)" ]; then
      if [ -z "$QUIET" ]; then
        echo "CLEANUP: Include folder is empty, removing it."
      fi 
      rm -rf include
    fi
  fi

  if [ -n "$QUIET" ] && [ -z "$REALLY_QUIET" ]; then
    echo "CLEANUP: Complete"
  fi


  # Move back to the previous location
  cd "$current_dir"
}

# This function is to build the "include" headers
# used for external projects
build_include() {
  if [ -n "$QUIET" ] && [ -z "$REALLY_QUIET" ]; then
    echo "BUILD INCLUDE: Starting"
  fi

  if [ ! -d "$1/include" ]; then
    if [ -z "$QUIET" ]; then
      echo "BUILD INCLUDE: Created include folder at $1/include."
    fi
     mkdir "$1/include"
  fi

  # Create the astera include folder
  if [ ! -d "$1/include/astera" ]; then
      mkdir "$1/include/astera"
  fi

  if [ -z "$QUIET" ]; then
    echo "BUILD INCLUDE: copying astera headers."
  fi
  cp -r "$1"/src/*.h "$1"/include/astera/ 

  # Copy over GLFW's headers
  if [ -d "$1"/dep/glfw ]; then
    if [ -z "$QUIET" ]; then
     echo "BUILD INCLUDE: copying GLFW headers."
    fi

    cp -r "$1"/dep/glfw/include/* "$1"/include/
  fi  


  # Copy over OpenAL-Soft's headers
  if [ -d "$1"/dep/openal-soft ]; then
    if [ -z "$QUIET" ]; then
     echo "BUILD INCLUDE: copying OpenAL-Soft headers."
    fi

    cp -r "$1"/dep/openal-soft/include/* "$1"/include/
  fi  

  if [ -n "$QUIET" ] && [ -z "$REALLY_QUIET" ]; then
    echo "BUILD INCLUDE: Complete"
  fi

}

build_examples() {
  echo "BUILD INFO: Build Examples to be implemented."
}

# Stop if something fails
set -e

# Parse thru all the options
while getopts ":hardniecsxqq" opt; do
  case $opt in
    h)
      echo "Usage ./build_unix.sh [-hardiecs]"
      echo "-h  Show this info"
      echo "-a  Build all (deps, headers, examples)"
      echo "-r  Build release (DEFAULT, optimizations, -O2)"
      echo "-d  Build debug (-g, no optimmizations)"
      echo "-i  Build include headers"
      echo "-e  Build examples"
      echo "-c  Build clean (remove previous build)"
      echo "-x  Don't build astera"
      echo "-q  Quiet"
      echo "-qq Really Quiet"
      exit 0
      ;;
    a)
      BUILD_ALL="1"
      ;;
    r)
      BUILD_RELEASE="1"
      ;;
    d)
      BUILD_DEBUG="1"
      ;;
    i)
      BUILD_INCLUDE="1"
      ;;
    e)
      BUILD_EXAMPLES="1"
      ;;
    c)
      BUILD_CLEAN="1"
      ;;
    x)
      NO_ASTERA="1"
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

# Check if we should build the includes as well 
if [ -n "$BUILD_ALL" ] || [ -n "$BUILD_INCLUDE" ]; then
  build_include "$ROOT_DIR"
fi

if [ -z "$NO_ASTERA" ]; then
  # If we're not supposed to rebuild
  if [ -n "$NO_REBUILD" ]; then
    # Make sure that any previous builds don't exist
   if [ ! -d "$ROOT_DIR/build" ]; then

    if [ -z "$REALLY_QUIET" ]; then
      echo "BUILD INFO: Generating Makefiles"
    fi
    
    # Otherwise, remake it
    if [ -z "$QUIET" ]; then
      cmake -S. -Bbuild
    else 
      cmake -S. -Bbuild >> /dev/null 2>&1
    fi
   fi
  else
    if [ -z "$REALLY_QUIET" ]; then
      echo "BUILD INFO: Generating Makefiles"
    fi

    # Generate the library's Makefile
    if [ -z "$QUIET" ]; then
      cmake -S. -Bbuild
    else 
      cmake -S. -Bbuild >> /dev/null 2>&1
    fi

  fi

  if [ -z "$REALLY_QUIET" ]; then
    echo "BUILD INFO: Building Libraries"
  fi

  # Generate the libraries with whatever generation tool targetted
  if [ -z "$QUIET" ]; then
    cmake --build build
  else 
    cmake --build build >> /dev/null 2>&1
  fi

  if [ -z "$REALLY_QUIET" ]; then
    echo "BUILD INFO: Astera built"
  fi
fi

# Build all the examples
if [ -n "$BUILD_ALL" ] || [ -n "$BUILD_EXAMPLES" ]; then 
  # TODO Examples Build implementation
  echo "BUILD INFO: Building examples"
  build_examples "$ROOT_DIR"
fi
