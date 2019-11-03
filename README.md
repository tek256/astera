<div id="header">
    <p align="center">
      <img width="64px" height="64px" style="border-radius: 6px;" src="res/tex/icon.png"><br>
      <b>astera</b><br>
	  <span font-size="16px">a cross platform game library</span><br>
      <span font-size="12px">Made by <a href="http://tek256.com">Devon</a> with love.</span><br><br>
      <span><a href="https://github.com/tek256/astera/wiki/Getting-Started/">Getting Started</a> | <a href="https://github.com/tek256/astera/tree/master/tool">Tools</a> | <a href="https://discordapp.com/invite/63GvpMh">Discord</a> | <a href="https://github.com/sponsors/tek256">Support</a></span>
    </p>
</div>
<div id="about">
	<h3>About</h3>
	<p>Astera is an in-development game library. The goal is to show how to make games and game engines work with lower level languages. Currently most of the major systems are up and running, but others are in development. Astera will eventually aim to be as dependency-free as possible. Using as low level of dependencies at official release as possible.  
	</p>
</div>
<div id="building">
<h3>Building</h3>
<p>Please note that at this stage, the library is not complete. You can try automated building by either running unix_build or win_setup.ps1 in your operating system's terminal emulator. If you have GLFW & OpenAL-Soft installed locally, you can simply run the Makefile, i.e <code>make</code> or <code>mingw32-make</code> for Windows systems.
</div>
<div id="contributing">
	<h3>Contributing</h3>
	<pre><code>Here are some things you can do to help contribute!
- Bug Review & Search
- Wiki / Code Documentation
- Example Creation</code></pre> 
</div>
<div id="dependencies">
<h3>Dependencies</h3>
<ul>
<li><a href="https://github.com/glfw/glfw">GLFW</a></li>
<li><a href="https://github.com/rxi/microui">MicroUI</a></li>
<li><a href="https://github.com/kcat/openal-soft">OpenAL-Soft</a></li>
<li><a href="https://github.com/nothings/stb/">STB</a> Image, Truetype, Vorbis</li>
<li><a href="https://github.com/kuba--/zip">ZIP</a></li>
<li><a href="https://github.com/Dav1dde/glad">GLAD</a></li>
<li><a href="https://github.com/benhoyt/inih">inih</a></li>
</ul>
</div>
<div id="changelog">
<h3>Change Log</h3>
<pre><code>Oct 25, 2019
- Cleaning up dependencies
- Updated Makefile to reflect dependency changes
- Updated README to be a bit more useful
- Removed phys.c/h to be re-added when completed
Oct 24, 2019
- Removing level.c/h until functional
- Updating to comply with pedantic compilation
- Buttoning up some semantics
- Refactoring to have consistent API naming
- Really wanting chicken strips
- Moving to C11 for anonymous union support
- Working on CI Support in other repository
Oct 21, 2019
- Working on render cache
- UI implementation work
Oct 10, 2019
- Update ZIP Dependency
- Refactoring into more legible code
- Prepping some files for proper library release
- Finishing UI Implementation
- Audio cleanup fix
- Writing include options / toggles
- Documenting parts of the engine in the wiki
- Transitioning to clang/LLVM workflow
Sept 24, 2019
- Added ZIP Functionality
- Moved CONTRIBUTING.md & LICENSE into README.md
- Initial Update of Audio Push System
- Migrating UI System into ui.c/h
</code></pre>
</div>
<div id="todo">
<h3>To Do</h3>
<pre><code>UI
- Rendering
- Gamepad / Keyboard Support
Audio  
- Seek Test
Game Rendering
- Particle Effects
- Framebuffer Effects
- Move to cached / managed assets
- Patch shader cache system to be dynamic
Level System
- Runtime Management
- Serialization 
- Loading from file
Physics
- All of it
Tooling
- Level Editor
- Build optimization
Documentation
- API Documentation
- Technical Breakdowns of each system
- Examples
Release 
- Library style refactor
Optional
- Multi Language Support (wide characters)
- Configuration Editor documentation
- Integrate minimal/own versions of dependencies
- World Domination OR just some chicken strips.
</code></pre>
</div>
<div id="license">
	<h3>License</h3>
	<p>The Unlicense</p>
<pre><code>This is free and unencumbered software released into the public domain.

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

For more information, please refer to <http://unlicense.org></code></pre></div>
