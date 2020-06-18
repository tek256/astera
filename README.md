<div id="header">
    <p align="center">
      <img width="64px" height="64px" style="border-radius: 6px;" src="examples/resources/textures/icon.png"><br>
      <b>astera</b><br>
	  <span font-size="16px">a cross platform game library</span><br>
      <span font-size="12px">Made by <a href="http://tek256.com">Devon</a> with love.</span><br><br>
      <span><a href="https://tek256.com/astera/Build%20Guide.html">Setup</a> | <a href="https://github.com/tek256/astera/tree/master/examples/">Examples</a> | <a href="https://discordapp.com/invite/63GvpMh">Discord</a> | <a href="https://github.com/sponsors/tek256">Support</a></span><br><br>
      <span><img src="https://github.com/tek256/astera/workflows/Build%20Astera/badge.svg"></span>
    </p>
</div>

### About
Astera is a game library focused on performance, portability, and maintainability. Astera aims to rely on as few dependencies as possible.  

#### Latest Release: 0.01  

### Target Platforms
Here are the target platforms for this library:  
```
- Windows
- Linux
- Mac OSX (versions supporting OpenGL 3.3+)
- BSD
``` 

_NOTE: I'm always open to adding more target platforms!_

### Libraries Used
[GLFW](https://github.com/glfw/glfw), [OpenAL-Soft](https://github.com/kcat/openal-soft), [GLAD](https://github.com/Dav1dde/glad), [STB](https://github.com/nothings/stb/) Image & Vorbis, [nanovg](https://github.com/memononen/nanovg), and [ZIP](https://github.com/kuba--/zip).

### Building
Astera and it's dependencies are built using CMake. You can run your own CMake command or try using one of the automated build scripts (they're located in the `tools/` folder).

```
Usage build_unix.sh && build_win.bat [-hrcxqq]
-h  Show this info
-r  Build release (optimizations, -O2)
-c  Build clean (remove previous build generated by this script)
-x  Don't build examples
-q  Quiet output
-qq Silence all output
Windows Only:
-m  Force use MinGW (gcc/g++)
-l  Force use LLVM (clang/clang++)
```

Example script usage (generate a release candidate):

```
./build_unix.sh -r
```

Example CMake usage:

```
cmake -Bbuilld -S. -DASTERA_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release # Generate the build files
cmake --build build # Build the build files
```

For more information see the relevant [wiki page](https://tek256.com/astera/Build%20Guide.html) or the `docs/Build Guide.rst` file.

### Changelog
```
June 18, 2020 - [0.01 RELEASE]
- New context based engine usage
  - System prefix + `_ctx` for audio, render, input, and ui systems
- Lots of bug fixes 
- Lots of code hardening 
- Pedantic fixes
- Basic Game, Audio, Input, and UI Examples created

May 8, 2020
- Static building
- Working on examples
- Refactored UI to context based usage (non-global) 

For older changes see docs/changelog-archive.txt
```

### Special Thanks
[Isabella Muerte](https://github.com/slurps-mad-rips) for being a patient friend and helping create the build system.  
[Sharlock93](https://github.com/sharlock93) for helping diagnose issues.  
[Dan Bechard](https://github.com/dbechrd) for helping test on Windows & discuss ideas with.  

