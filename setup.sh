#!/bin/sh

echo "Starting setup."
cd dep

echo "Cloning & Building Dependencies"

echo "Cloning GLFW"
git clone https://github.com/glfw/glfw.git
cd glfw
echo "Building GLFW"
cmake -G "Unix Makefiles"
make
sudo make install
cd ..

echo "Cloning OpenAL-Soft"
git clone https://github.com/kcat/openal-soft
cd openal-soft
echo "Building OpenAL-Soft"
cmake -G "Unix Makefiles"
make
sudo make install
cd ..

echo "Cloning miniz"
git clone https://github.com/richgel999/miniz
cd miniz
echo "Building miniz"
cmake -G "Unix Makefiles"
make
sudo make install
cd ../..
