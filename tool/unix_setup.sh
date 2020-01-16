#!/bin/sh
# Written by Devon, Github @ tek256 

# Note: This setup involves installing the dependencies
# to the system path. If that's not something you want, 
# then run it as "unix_setup.sh dynamic"

if [ $1 == "dynamic" ]; then
  echo "Building Dependencies in Dynamic Mode"

  echo "##############"
  echo "Building GLFW."  
  echo "##############"
  cd ../dep/glfw
  cmake -G "Unix Makefiles"
  make

  echo "##############"
  echo "Building OpenAL-Soft"
  echo "##############"
  cd ../openal-soft/
  cmake -G "Unix Makefiles"
  make

  echo "##############"
  echo "Building Astera"
  echo "##############"
  cd ../../
  mkdir lib
  make
else
  echo "Requesting sudo privileges to install dependencies."
  if [ $EUID != 0 ]; then
    sudo "$0" "$@"
    exit $?
  fi

  echo "##############"
  echo "Building GLFW."  
  echo "##############"
  cd ../dep/glfw
  cmake -G "Unix Makefiles"
  make
  sudo make install

  echo "##############"
  echo "Building OpenAL-Soft"
  echo "##############"
  cd ../openal-soft/
  cmake -G "Unix Makefiles"
  make
  sudo make install

  cd ../../
  mkdir lib
  make
fi
