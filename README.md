<div id="header">
    <p text-align="center">
      <img width="64px" height="64px" style="border-radius: 6px;" src="res/tex/icon.png"><br>
      <b>astera</b><br>
	  <span font-size="16px">a cross platform game library</span><br>
      <span font-size="12px">Made by <a href="http://tek256.com">Devon</a> with love.</span><br><br>
      <span><a href="https://github.com/tek256/astera/wiki/Building/">Building</a> | <a href="https://github.com/tek256/astera/tree/master/tool">Tools</a> | <a href="https://discordapp.com/invite/63GvpMh">Discord</a> | <a href="https://github.com/sponsors/tek256">Support</a></span>
    </p>
</div>

### About
Astera is a game library focused on performance, portability, and maintainability. The goal is to show how to make games and game engines work with lower level languages. Astera aims to rely on as few dependencies as possible.  

### Building
Please note that at this stage, the library is not at a stable release candidate. You can try automated building by either running `unix_setup.sh` or `win_setup.ps1` in your terminal. If you have GLFW & OpenAL-Soft installed locally, you can simply run the Makefile, i.e `make` or `mingw32-make` for MinGW systems.

For more information see the relevant [wiki page](https://github.com/tek256/astera/wiki/Building).

### Libraries Used
Note: All dependencies are bundled with Astera. You should be able to run thru the build process relatively easily.
[GLFW](https://github.com/glfw/glfw), [OpenAL-Soft](https://github.com/kcat/openal-soft), [STB](https://github.com/nothings/stb/) Image, Truetype, Vorbis, [ZIP](https://github.com/kuba--/zip), [nanovg](https://github.com/memononen/nanovg), and [GLAD](https://github.com/Dav1dde/glad)  

### Target Platforms
Here are the target platforms for this library:  
```
- Linux (distro agnostic)
- Windows XP+
- Mac OSX (version supporting OpenGL 3.3+)
- BSD (Free, Net, Open)
- Raspberry Pi (0+)
```  
These are some potential future targets:  
```
-Android
-Nintendo Switch
-iOS
```

### Changelog
```
Jan 16, 2020
- Prepping for 0.01 Release
- Rendering changes
- Audio refactor
- UI System in progress
Dec 14, 2019 - 2
- Audio music working
- Updated vorbis to 1.17
Dec 14, 2019
- Subsprite bugfix
- Working on audio update
- Rework to most pedantic standards
- Updated a few `tool` programs
- Removing unused timer implementation, potentially replaced in the future
- Updated all time implementations to be decimal milliseconds
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
