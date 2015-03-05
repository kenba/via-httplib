# Make #
## Tools, and their versions, currently built against##
- CMake version 2.8
- OpenSSL version 1.0.1j
- Boost version 1.57.0
- MinGW version 4.9.1
- Visual Studio 2013

# Recommended build steps #
Set the environment variable OPENSSL\_ROOT\_DIR to the location of where **OpenSSL** has been installed.

Set the environment variable BOOST\_ROOT to the location of where **Boost** has been installed.

## Building for MinGW ##
### Debug ###
From the via-httplib folder create a directory, say mak-mingw-Debug.
Run the commands:

	cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CROSS_COMPILING=ON \
	-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DVIA_HTTPLIB_BUILD_SHARED_LIBS=OFF ...

	mingw32-make

###Release###
From the via-httplib folder create a directory, say mak-mingw-Release.
Run the commands

	cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CROSS_COMPILING=ON \
	-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DVIA_HTTPLIB_BUILD_SHARED_LIBS=OFF ...

	mingw32-make

## Building for Visual Studio 2013 ##
From the via-httplib folder create a directory, say mak-msvc2013.
Run the command

	cmake -G "Visual Studio 2013" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl \
	-DVIA_HTTPLIB_BUILD_SHARED_LIBS=OFF ...

### Debug ###
Run the command

	msbuild VIA-HTTPLIB.sln /p:Configuration="Debug"

### Release ###
Run the command

	msbuild VIA-HTTPLIB.sln /p:Configuration="Release"
