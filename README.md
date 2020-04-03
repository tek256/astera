<div id="header">
    <p align="center">
      <img width="64px" height="64px" style="border-radius: 6px;" src="docs/icon.png"><br>
      <b>astera</b><br>
	  <span font-size="16px">a cross platform game library</span><br>
      <span font-size="12px">Made by <a href="http://tek256.com">Devon</a> with love.</span><br><br>
      <span><a href="https://github.com/tek256/astera/wiki/Setup">Setup</a> | <a href="https://github.com/tek256/astera/tree/master/docs/examples/">Examples</a> | <a href="https://discordapp.com/invite/63GvpMh">Discord</a> | <a href="https://github.com/sponsors/tek256">Support</a></span>
    </p>
</div>

### About
Astera is a game library focused on performance, portability, and maintainability. The goal is to show how to make games and game engines work with lower level languages. Astera aims to rely on as few dependencies as possible.  

#### Latest Release: 0.01-PRE

### Building
Please note that at this stage the library is not at a stable release candidate. You can try automated building by either running the `build_unix.sh` or `build_win.bat` script in the root directory. 

Both of the build scripts have the same arguments.
```
Usage build_unix.sh && build_win.sh [-hardiecsnxqq]
-h  Show this info
-a  Build all (deps, headers, examples)
-r  Build release (DEFAULT, optimizations, -O2)
-d  Build debug (-g, no optimmizations)
-i  Build include headers
-e  Build examples
-c  Build clean (remove previous binarys / builds)
-s  Build static library
-n  Don't rebuild dependencies
-x  Don't build astera
-q  Quiet output
-qq Really quiet output
```

For more information see the relevant [wiki page](https://github.com/tek256/astera/wiki/Setup) or the `docs/setup.md` file.

### Libraries Used
[GLFW](https://github.com/glfw/glfw), [OpenAL-Soft](https://github.com/kcat/openal-soft), [GLAD](https://github.com/Dav1dde/glad), [STB](https://github.com/nothings/stb/) Image & Vorbis, [nanovg](https://github.com/memononen/nanovg), and [ZIP](https://github.com/kuba--/zip)(optional).

### Target Platforms
Here are the target platforms for this library:  
```
- Linux (distro agnostic)
- Windows XP+
- Mac OSX (versions supporting OpenGL 3.3+)
- BSD (Free, Net, Open)
```  
These are some potential future target platforms:  
```
- Raspberry Pi (0+)
- Nintendo Switch
- Android
- iOS
```

### Future Features 
These are some potential future features: 
```
- OpenGL ES 2.0 Support
```

### Changelog
```
Mar 21, 2020
- Updated OpenAL-Soft to 1.20.1
- AABB & Circle Collision detection working
- Audio Effects completed
- Audio Distance Calculations done
- In progress: Quad trees & level system
Mar 18, 2020
- Build Script (Unix only for now, Windows in the works)
- Recreated useful tools
- Dynamic Compilation Support
- Static Baked Sheets (layers)
- PAK File Support
- Audio Effects (in the works)
- Level System (in the works)
- Rendering Bug Fixes
- Probably missed a lot of things here
- Added GitHub Workflows (CI) Support
Feb 5, 2020
- Largely Documentation
- Converted Documentation to Markdown & Legible formatting
- Moved `examples/` into `docs/`
- Updated scripts in `tool` for usability & legibility
Feb 3, 2020
- 0.01 PRE-Release
- Asset Streaming changes
- Wave file playing
- Pedantic cleaning
- UI Finished
For older changes see docs/changelog-archive.txt
```

### License  
The Unlicense 
```
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
```

