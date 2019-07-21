# Windows Build Environment

## Assumptions

This guide assumes you're building on a Windows 10 desktop and have the ability to run Powershell with administrative privileges.

## Prerequisites

* MinGW ([Graphical Installer](https://osdn.net/projects/mingw/downloads/68260/mingw-get-setup.exe))
  * This guide assumes MinGW is installed at `C:\MinGW`
  * In the installer, make sure *also install support for the graphical user interface* is selected, and for convenience also select at least one of *in the start menu* and/or *on the desktop*. ([Example](https://i.imgur.com/og5hlU9.png))
  * Once installed, the MinGW Installation Manager will start. With *Basic Setup* selected on the left, mark the following packages for installation (click the box, choose *Mark for installation*) and then choose *Apply Changes* from the *Installation* menu ([Example](https://i.imgur.com/ycLqDzW.png)):
    * mingw-developer-toolkit-bin
    * mingw32-base-bin
    * mingw32-gcc-g++-bin (required to build openal-soft)
    * msys-base-bin
* CMake ([32-bit](https://github.com/Kitware/CMake/releases/download/v3.15.0/cmake-3.15.0-win32-x86.msi), [64-bit](https://github.com/Kitware/CMake/releases/download/v3.15.0/cmake-3.15.0-win64-x64.msi))
  * This guide assumes CMake is installed at `C:\Program Files\CMake`
* A git command line client ([Installer](https://git-scm.com/download/win)) available on the path

## Acquiring and Building

* Open a Powershell with Administrative privileges
* Check out the engine (this guide assumes the code will live in `C:\Code\engine`)
  * `c:`
  * `cd \Code`
  * `git clone https://github.com/tek256/engine`
  * `cd engine`
* *READ* the header of the `setup.ps1` script and follow the directions there to build the dependencies and set up your path correctly.
* Once your dependencies are built and your path is set up, you can build the engine by typing: `mingw32-make`
