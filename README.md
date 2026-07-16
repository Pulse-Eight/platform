![Pulse-Eight logo](https://pulseeight.files.wordpress.com/2016/02/pulse-eight-logo-white-on-green.png?w=200)

# About
This library provides platform specific support for other libraries. It was written before
C++11, to supply the threading, atomics, timers, sockets and string helpers that the
standard library did not have at the time. It is used by libCEC, and was used by binary
add-ons for Kodi.

> ## Maintenance mode — use `std::` instead
>
> **Don't use this library in new projects.** Everything it was written to provide is in
> the standard library now — `std::mutex`, `std::recursive_mutex`,
> `std::condition_variable`, `std::thread`, `std::chrono`, `std::atomic` — and the standard
> library does it better. Use it directly and skip the dependency.
>
> [2.2.0](https://github.com/Pulse-Eight/platform/releases/tag/p8-platform-2.2.0) is the
> last planned release. libCEC, the only actively developed user left, is moving to `std::`
> directly. The Kodi binary add-ons already did.
>
> **Maintenance mode is not abandoned.** This repository stays open, fixes are still
> welcome and will still be merged, and nothing installed today stops working. There is
> simply no further development planned.

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
