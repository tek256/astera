<div id="header">
    <p align="center">
      <img width="64px" height="64px" style="border-radius: 6px;" src="res/tex/icon.png"><br>
      <b>astera</b><br>
	  <span font-size="16px">a cross platform game library</span><br>
      <span font-size="12px">Made by <a href="http://tek256.com">Devon</a> with love.</span><br><br>
      <span><a href="https://github.com/tek256/astera/wiki/Building/">Building</a> | <a href="https://github.com/tek256/astera/tree/master/tool">Tools</a> | <a href="https://discordapp.com/invite/63GvpMh">Discord</a> | <a href="https://github.com/sponsors/tek256">Support</a></span>
    </p>
</div>
<div id="about">
	<h3>About</h3>
	<p>Astera is an in-development game library. The goal is to show how to make games and game engines work with lower level languages. Currently most of the major systems are up and running, but others are in development. Astera aims to rely on as few dependencies as possible. Using as low level of dependencies at official release as possible.  
	</p>
</div>
<div id="building">
<h3>Building</h3>
<p>Please note that at this stage, the library is not at a stable release candidate. You can try automated building by either running <code>unix_build</code> or <code>win_setup.ps1</code> in your operating system's terminal emulator. If you have GLFW & OpenAL-Soft installed locally, you can simply run the Makefile, i.e <code>make</code> or <code>mingw32-make</code> for Windows systems.

  For more information see the relevant <a href="https://github.com/tek256/astera/wiki/Building">wiki page</a>.
</div>
<div id="dependencies">
<h3>Dependencies</h3>
<p><a href="https://github.com/glfw/glfw">GLFW</a>, <a href="https://github.com/kcat/openal-soft">OpenAL-Soft</a>, <a href="https://github.com/nothings/stb/">STB</a> Image, Truetype, Vorbis, <a href="https://github.com/kuba--/zip">ZIP</a>, and <a href="https://github.com/Dav1dde/glad">GLAD</a></p>
</div>
<div id="targets">
<h3>Target Platforms</h3>
<p>Here are the target platforms for this library:</p>
<pre><code>-Linux (distro-agnostic)
-Windows XP, 7, 10+
-Mac OSX (versions that support OpenGL 3.3)
-BSD (Free, Net, Open)
</code></pre>
<p>These are some potential future targets:</p>
<pre><code>-Android
-Nintendo Switch
-iOS
-Raspberry Pi (0,3B+) - Linux/BSD
</code></pre>
</div>
<div id="changelog">
<h3>Change Log</h3>
<pre><code>Dec 14, 2019 - 2
- Audio music working
- Updated vorbis to 1.17
Dec 14, 2019
- Subsprite bugfix
- Working on audio update
- Rework to most pedantic standards
- Updated a few `tool` programs
- Removing unused timer implementation, potentially replaced in the future
- Updated all time implementations to be decimal milliseconds
Dec 4, 2019
- Starting new layout of files for library style release
- Framebuffers completed
- Remove INIH completely
- Rewrote core loop timing example in main.c
- Added render "stress" test, just 2048 quads.
For older changes see docs/changelog-archive.txt
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
