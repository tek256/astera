#!/bin/sh
echo "Starting setup. \n Building Dependencies."
cd dep/glfw
echo "Building GLFW."
cmake -G "Unix Makefiles"
make
sudo make install
cd ../openal-soft
echo "Building OpenAL-Soft"
cmake -G "Unix Makefiles"
make
sudo make install
cd ../miniz
echo "Building miniz"
cmake -G "Unix Makefiles"
make
sudo make install
cd ../..
make
