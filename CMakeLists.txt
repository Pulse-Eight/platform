project(p8-platform)

cmake_minimum_required(VERSION 2.8.9)
enable_language(CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Threads REQUIRED)
include(UseMultiArch.cmake)
include(CheckAtomic.cmake)

set(p8-platform_NAME p8-platform)
set(p8-platform_DESCRIPTION "Pulse-Eight platform support library")
set(p8-platform_VERSION_MAJOR 2)
set(p8-platform_VERSION_MINOR 1)
set(p8-platform_VERSION_PATCH 1)

set(CMAKE_POSITION_INDEPENDENT_CODE on)

if(WIN32)
  set(PLAT_SOURCES src/windows/dlfcn-win32.cpp
                   src/windows/os-threads.cpp)
endif()

set(p8-platform_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include/p8-platform")
if(WIN32)
  LIST(APPEND p8-platform_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include/p8-platform/windows")
endif(WIN32)
set(p8-platform_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

if(NOT ${CORE_SYSTEM_NAME} STREQUAL "")
  if(${CORE_SYSTEM_NAME} STREQUAL "osx" OR ${CORE_SYSTEM_NAME} STREQUAL "ios")
    list(APPEND p8-platform_LIBRARIES "-framework CoreVideo")
  endif()
endif()

set(SOURCES src/util/StringUtils.cpp)

add_library(p8-platform ${SOURCES} ${PLAT_SOURCES})
target_link_libraries(p8-platform ${p8-platform_LIBRARIES})
set_target_properties(p8-platform
    PROPERTIES
    VERSION ${p8-platform_VERSION_MAJOR}.${p8-platform_VERSION_MINOR}.${p8-platform_VERSION_PATCH}
    SOVERSION ${p8-platform_VERSION_MAJOR})

if(WIN32)
  if (MSVC)
    # generate pdb in release mode too
    set_target_properties(p8-platform
        PROPERTIES
        COMPILE_PDB_NAME_DEBUG p8-platform${CMAKE_DEBUG_POSTFIX}
        COMPILE_PDB_NAME_RELEASE p8-platform
        COMPILE_PDB_NAME_MINSIZEREL p8-platform
        COMPILE_PDB_NAME_RELWITHDEBINFO p8-platform)

    if (${WIN64})
      # default setting that got removed in recent vs versions, generates a warning if set
      string(REPLACE "/arch:SSE2" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    endif(${WIN64})

  endif(MSVC)

  if ((NOT ${WIN64}) AND (NOT ${_M_ARM64}))
    add_definitions(-D_USE_32BIT_TIME_T)
  endif((NOT ${WIN64}) AND (NOT ${_M_ARM64}))
endif(WIN32)

install(TARGETS p8-platform DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES src/os.h DESTINATION include/p8-platform)
if(WIN32)
  install(FILES src/windows/dlfcn-win32.h
                src/windows/os-socket.h
                src/windows/os-threads.h
                src/windows/os-types.h
          DESTINATION include/p8-platform/windows)
else(WIN32)
  install(FILES src/posix/os-socket.h
                src/posix/os-threads.h
                src/posix/os-types.h
          DESTINATION include/p8-platform/posix)
endif(WIN32)
install(FILES src/sockets/cdevsocket.h
              src/sockets/socket.h
              src/sockets/tcp.h
        DESTINATION include/p8-platform/sockets)
install(FILES src/threads/atomics.h
              src/threads/mutex.h
              src/threads/threads.h
        DESTINATION include/p8-platform/threads)
install(FILES src/util/atomic.h
              src/util/buffer.h
              src/util/StringUtils.h
              src/util/StdString.h
              src/util/timeutils.h
              src/util/util.h
        DESTINATION include/p8-platform/util)

if(MSVC)
  # install generated pdb
  install(FILES $<TARGET_FILE_DIR:p8-platform>/p8-platform.pdb
          DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif(MSVC)

if(NOT WIN32)
  configure_file(p8-platform.pc.in p8-platform.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/p8-platform.pc
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif(NOT WIN32)

# config mode
configure_file (p8-platform-config.cmake.in
                p8-platform-config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/p8-platform-config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/p8-platform)

