# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Read this first: use `std::`, not this library

p8-platform was written before C++11 to provide threading, atomics, timers, sockets and
string helpers the standard library didn't have yet. **It does now.** As of **2.2.0** this
library is in **maintenance mode** and 2.2.0 is the last planned release.

**Never suggest this library for new code.** The mapping is one-to-one, and `std::` wins
every time:

| p8-platform | use instead |
|---|---|
| `CMutex` | `std::recursive_mutex` (it *is* recursive — see below) |
| `CLockObject` | `std::unique_lock` / `std::lock_guard` |
| `CCondition` | `std::condition_variable` / `std::condition_variable_any` |
| `CEvent` | `std::condition_variable` + a flag |
| `CThread` | `std::thread` / `std::jthread` |
| `CTimeout`, `timeutils.h` | `std::chrono::steady_clock` |
| `atomics.h`, `util/atomic.h` | `std::atomic` |
| `StringUtils`, `CStdString` | `std::string` + `<algorithm>` |
| sockets, `dlfcn-win32` | no std equivalent — the only part with a reason to exist |

libCEC is the only actively developed consumer left and is moving to `std::` directly. The
Kodi PVR add-ons already did — their changelogs literally say *"Remove p8-platform
dependency"* and *"use std::mutex"*.

**Maintenance mode is not abandoned.** Fixes are welcome and get merged. Don't break
consumers, don't archive it, don't start a rewrite. The bar for a change here is "someone
is broken", not "this could be nicer".

## What this is

A small C++11 support library (`libp8-platform.so` / `p8-platform.lib`, SONAME
`libp8-platform.so.2`). Consumers link the copy **installed on the system**
(`find_package(p8-platform)`; Debian's `libp8-platform-dev`). Almost all of it is
**header-only inline code** under `src/`, installed to `<prefix>/include/p8-platform/`.

`src/threads/` mutex, condition, event, thread · `src/util/` StringUtils, buffer, timeutils,
StdString, atomics · `src/sockets/` tcp, cdevsocket · `src/posix/` + `src/windows/` the
per-OS split · `src/os.h` the entry header.

## Building

```
mkdir build && cd build
cmake ..
make -j4
sudo make install && sudo ldconfig    # no ldconfig on OS X
```

Windows uses `windows/build.cmd` (VS2022), which needs the `support` submodule
(`git submodule update --init --recursive`). Nothing else invokes those `.cmd` scripts —
libCEC's own Windows build drives this tree through its `create-installer.py` instead.

There is **no test suite**. Verification is: build it, then build libCEC against it and run
`cec-client -l` against real hardware.

## Traps that have burned people here

These are measured, not theoretical. Check them before believing a green build.

- **The build compiles exactly one file.** `set(SOURCES src/util/StringUtils.cpp)`. Everything
  else is header-only and **never exercised by this project's own build**. `make` succeeding
  proves almost nothing — the library builds clean at C++11/14/17/20 while its headers emit
  11 warnings at C++20 (`register`, removed in C++17; deprecated `volatile` arithmetic). To
  test a header change, compile the headers from a *consumer*, not from here.
- **gcc silently suppresses warnings from `/usr/local/include`**, the default install prefix.
  A header can be emitting warnings at every consumer for years and look clean locally.
  Install to a non-system prefix to see them.
- **`CMutex` is recursive** (`pthread_mutexattr_settype(..., PTHREAD_MUTEX_RECURSIVE)`;
  `CRITICAL_SECTION` is recursive too). Anything mapping it to `std::mutex` is wrong.
- **`mutex_t` is a different kind of thing per platform**: `pthread_mutex_t` **by value** on
  posix, `CRITICAL_SECTION*` — `new`'d and `delete`'d — on Windows. Any change touching
  `CMutex` lifetime must handle both; a naive move double-deletes on Windows and corrupts
  on posix.
- **`CMutex::Clear()` has no std equivalent.** It fully unwinds a recursive lock, which
  `std::recursive_mutex` can't do (it can't report its own recursion count). The manual
  `m_iLockCount` exists for this. It also looks buggy — `TryLock()` increments the count,
  then it unlocks that many times. Don't "fix" it in passing.
- **Declaring a destructor suppresses the implicit move**, whatever the base class says.
  `PreventCopy` allows moves, but every subclass declares a destructor, so none of them are
  movable. Adding moves to the base changes nothing (this was issue #33).
- **`IMPORTED_LINK_INTERFACE_LANGUAGES` is ignored unless the imported library is `STATIC`**
  *and* the consumer has enabled CXX. Setting it on an `UNKNOWN IMPORTED` target is a
  silent no-op (this was issue #35).
- **These are public installed headers.** Changing one is an API change even when it looks
  internal. The consumer set is small but real: libCEC and its forks, ~15 distro/build
  recipes, and a tail of dormant Kodi add-ons.

## Conventions

- **Commits**: lowercase `fixed:` / `added:` / `changed:` subjects. Explain *why* in the body.
  Credit contributors by handle. No Co-Authored-By trailers.
- **Comments**: minimal. Only state what the code can't — a constraint, a platform asymmetry,
  a standards trap. No ticket references in code.
- **Generated files**: `p8-platform.pc` and `p8-platform-config.cmake` come from the `.in`
  templates. Edit the template, never the generated file.
- **The installed cmake config must stay relocatable** — derive paths from
  `CMAKE_CURRENT_LIST_DIR`, never bake in `CMAKE_INSTALL_PREFIX`. It previously resolved to
  whichever p8-platform turned up first on the search path, silently linking a different
  library (issue #38).
- **Use `GNUInstallDirs`**, never hand-rolled libdir logic. A `set(... CACHE PATH ...)` on a
  relative `-D` value makes it absolute against the build dir, which installs into the build
  tree (issue #41).
- **Version**: `CMakeLists.txt` (`p8-platform_VERSION_*`). The config must report the full
  `major.minor.patch`, or `find_package(p8-platform x.y.z)` can't gate on it.
