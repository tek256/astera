### Foreword
To get started with compiling the engine there is a setup script file provided. It is called simply, setup.

Both `GLFW` and `OpenAL-Soft` rely on [CMake](https://cmake.org/), so please insure that you have it installed.

Currently, Astera only builds via a standard `Makefile`. All of the current examples use it as well. If you're interested in porting it to a different build system, please feel free to make a fork!

- [Linux Setup](#linux-setup)
- [Windows Setup (MinGW)](#windows-setup-mingw)
- [BSD Setup](#bsd-setup)
- [Mac OSX Setup](#mac-osx-setup)
- [Make Options](#make-options)


### Linux Setup
If you run into any issues, please check that you have all the required packages installed and are running the commands from the correct directory within your terminal! To find a solution you can always check the [issues](https://github.com/tek256/astera/issues) page. 

#### 0. Dependencies

Before compiling, you'll want to insure that you have the following libraries available on your system:
_Note: Most of these should be available within your package manager's `xorg-dev` package._

```
-GL (OpenGL)
-GLU (OpenGL Utilities)
-Xi
-Xrandr
-Xxf86vm
-Xinerama
-Xcursor
```

To double check these dependencies, you can look at the `LINKER_FLAGS` line in one of the `Makefile`s. They should be clearly labelled!

#### 1. Library Setup

At this stage we just need to setup the `GLFW` and `OpenAL-Soft` dependencies, to accomplish this, you should be able to simply run the follow commands from the repository's root.

```
chmod +x tool/unix_setup
tool/unix_setup
```

If that doesn't work, simply repeat the following steps for both `dep/GLFW` and `dep/OpenAL-Soft`.
```
cd dep/GLFW
cmake -G "Unix Makefiles"
make
```

#### 2. Compiling
At this point all the resources for building with Astera should be set up. All you should have to do is run the following command from the repository's root. 

```
make
```

Below will be a listing of `make options` which will list out options for you to target a specific build from the repository.
Alternatively you can run the `Makefile` by calling your system's version of `make` within the directory of each example. 


### Windows Setup (MinGW)
Astera currently doesn't have a build path for Visual Studio or CMake, because of it the library currently relies on [MinGW](http://mingw.org/). If you're interesting in helping make a visual studio solution for astera, please open a fork!

#### 1. Library Setup
For much of this to work, Windows requires your powershell or command prompt to be `Run as Administrator`. 

To setup the `GLFW` and `OpenAL-Soft` dependencies, you can simply run the powershell script: `tool/win_setup.ps1`  

If the setup file fails, you can simply run the following commands for both `dep/glfw` and `dep/openal-soft`.

_Note: For the `cmake -G` step, you can use either "MinGW Makefiles" or "MSYS Makefiles"._
```
cd dep/glfw/
cmake -G "MinGW Makefiles"
mingw32-make
```

### 2. Compiling
At this point all the resources for building with Astera should be set up. All you should have to do is run the following command from the repository's root. 

```
mingw32-make
```

Below will be a listing of `make options` which will list out options for you to target a specific build from the repository.
Alternatively you can run the `Makefile` by calling your system's version of `make` within the directory of each example. 

_Note: This setup is written for the MinGW compiler suite on Windows currently. More ways of compiling will be added later!_



### BSD Setup
Under construction, and coming soon! 

### Mac OSX Setup
Under construction, and coming soon!


### Make Options
If you want to target a specific compiler for your system without using astera's default target compiler, you can run `make CC=compiler_here`  
More soon
