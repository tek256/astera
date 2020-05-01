### Foreword
In order to compile astera please make sure that you have a newer version of CMake (3.16+) installed.  

- [Setup](#setup)
- [Linux Dependencies](#linux-dependencies)
- [CMake Options](#cmake-options)

### Setup

In this step we're looking to initialize the CMake build environment. In order to do so, make sure you're in the root directory of astera in your terminal of choice! 

This is the barebones command you can run to get started. It will create the build directory in a new folder called build. The `-S.` part just specifies to CMake that the source's root directory is the root directory of the repository.
```
cmake -Bbuild -S.
```

For specific setup options, see [CMake Options](#cmake-options).  

Now that we have the CMake environment initialized, we just have to call the build function. 
For ease, you can call it from the root directory via:
```
cmake --build build
```

Everything compiled will be located within the `astera/build` directory now. 

### Linux Dependencies
If you run into any issues, please check that you have all the required packages installed and are running the commands from the correct directory within your terminal! To find a solution you can always check the [issues](https://github.com/tek256/astera/issues) page. 

Before compiling, you'll want to insure that you have the following libraries available on your system:
```
-GL (OpenGL, often via mesa)
-GLU (OpenGL Utilities)
-Xi
-Xrandr
-Xxf86vm
-Xinerama
-XCursor
```

Here are a few package lists for various Linux Distributions in case you need them installed:  

**Ubuntu**:
```
sudo apt install mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev
```

**Alpine Linux**: 
```
sudo apk add mesa-dev libxrandr-dev libx11-dev libxi-dev freeglu-dev libxinerama-dev libxcursor-dev
```

**Void Linux**:
```
sudo xbps-install MesaLib-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel
```

NOTE: If you have a package manager on here that you'd like to see added, please create an issue and add the package names if you know them!

### CMake Options

If you want to specify a CMake Generator, you can add `-G` and then your option, I'm partial to ninja, so I'd add `-GNinja`. You can read more about CMake Generators [here](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).

If you'd like to specify a specific compiler use `-DCMAKE_C_COMPILER=` and `-DCMAKE_CXX_COMPILER=` in the command.
For example using clang/clang++:
```
cmake -Bbuild -S. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
```
