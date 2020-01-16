# This script WILL NOT RUN unless your Execution Policy is set to 
# "Unrestricted" which allows your system to run PowerShell scripts that 
# aren't signed.  If you get an UnauthorizedAccess exception when trying
# to run this script, do the following:
# 
# 1) Open Powershell as an Administrator (left as an exercise to the reader)
# 2) Type: Set-ExecutionPolicy Unrestricted
# 3) The system will give you a warning. Read it, and if you understand and 
#      accept select Y.
# 4) Close your PowerShell session and re-open a new Powershell Administrator
#      session.
# 5) Run this script from the top of the engine source: & .\win-setup.ps1
# 6) Optional: Repeat steps 1-4 but change "Unrestricted" to "Restricted" if 
#      you'd like to restore your Execution Policy to the default setting.

# This sets up the path for the following sections. Change the following
# directories if you didn't install the prerequisites in the expected
# directories (see Getting Started in the Wiki)

# The following section will build the dependencies required to compile the
# engine.
#
# Script written by Mark W. Github @ markwhi

echo "#######################"
echo "########script#########"
echo "###written by mark w###"
echo ""
echo "#######################"
echo "Building openal-soft..."
echo "#######################"
cd ..\dep\openal-soft
cmake -G "MinGW Makefiles"
mingw32-make

echo "#######################"
echo "Building GLFW..."
echo "#######################"
cd ..\glfw
cmake -G "MinGW Makefiles"
mingw32-make

echo "Done."
echo "You should now be able to run mingw32-make to build astera."

