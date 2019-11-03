#!/bin/sh
cd ../dep/glfw
cmake -G "Unix Makefiles"
make
sudo make install
cd ../openal-soft/
cmake -G "Unix Makefiles"
make
sudo make install
cd ../zip/
cmake -G "Unix Makefiles"
make
sudo make install
cd ../../tool
make
