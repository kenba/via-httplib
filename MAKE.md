# Make #
## Tools, and their versions, currently built against##
- CMake version 2.8
- OpenSSL version 1.0.1c
- Boost version 1.51.0 or 1.52.0
- MinGW version 4.4.0
- Visual Studio 10.0

# Recommended build steps #
Set the environement variable OPENSSL\_ROOT\_DIR to the location of where **OpenSSL** has been installed.

Set the environement variable BOOST\_ROOT to the location of where **Boost** has been installed.

## Building for MinGW ##
### Debug ###
From the via-httplib folder create a directory, say mak-mingw-Debug.
Run the commands

*cmake -G "MinGW Makefiles" -DCMAKE\_BUILD\_TYPE=Debug -DCMAKE\_CROSS\_COMPILING=ON -DCMAKE\_C\_COMPILER=gcc -DCMAKE\_CXX\_COMPILER=g++ -DVIA\_HTTPLIB\_BUILD\_SHARED\_LIBS=OFF ..*

*mingw32-make*
###Release###
From the via-httplib folder create a directory, say mak-mingw-Release.
Run the commands

*cmake -G "MinGW Makefiles" -DCMAKE\_BUILD\_TYPE=Release -DCMAKE\_CROSS\_COMPILING=ON -DCMAKE\_C\_COMPILER=gcc -DCMAKE\_CXX\_COMPILER=g++ -DVIA\_HTTPLIB\_BUILD\_SHARED\_LIBS=OFF ..*

**mingw32-make**
## Building for Visual Studio 2010 ##
From the via-httplib folder create a directory, say mak-msvc2010.
Run the command

*cmake -G "Visual Studio 10" -DCMAKE\_C\_COMPILER=cl -DCMAKE\_CXX\_COMPILER=cl -DVIA\_HTTPLIB\_BUILD\_SHARED\_LIBS=OFF ..*
### Debug ###
Run the command

*msbuild VIA-HTTPLIB.sln /p:Configuration="Debug"*
### Release ###
Run the command

*msbuild VIA-HTTPLIB.sln /p:Configuration="Release"*

[Back](/README.md)