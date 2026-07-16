![Pulse-Eight logo](https://pulseeight.files.wordpress.com/2016/02/pulse-eight-logo-white-on-green.png?w=200)

# About
This library provides platform specific support for other libraries, and is used by libCEC and binary add-ons for Kodi

# Supported platforms

## Linux, BSD & Apple OS X
To compile this library, you'll need the following dependencies:
* [cmake 3.12.0 or better](https://www.cmake.org/)
* a supported C++ 11 compiler

On Debian and Ubuntu, `apt-get install git cmake build-essential` pulls in these
and the git client used below.

Follow these instructions to compile and install the library:
```
git clone https://github.com/Pulse-Eight/platform.git
mkdir platform/build
cd platform/build
cmake ..
make -j4
sudo make install
```

On Linux and BSD, run `sudo ldconfig` afterwards to refresh the linker cache.
OS X has no equivalent and doesn't need one.

## Microsoft Windows
To compile this library on Windows, you'll need the following dependencies:
* [cmake 3.12.0 or better](https://www.cmake.org/)
* [Visual Studio 2022](https://www.visualstudio.com/)

Follow these instructions to compile and install the library:
```
git clone https://github.com/Pulse-Eight/platform.git
cd platform
git submodule update --init --recursive
cd windows
build.cmd
```
