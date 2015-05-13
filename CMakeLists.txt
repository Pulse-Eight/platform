project(platform)

cmake_minimum_required(VERSION 2.8.9)
enable_language(CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Threads REQUIRED)
include(UseMultiArch.cmake)
include(CheckAtomic.cmake)

set(platform_NAME platform)
set(platform_DESCRIPTION "Platform support library")
set(platform_VERSION_MAJOR 1)
set(platform_VERSION_MINOR 0)
set(platform_VERSION_PATCH 9)

set(CMAKE_POSITION_INDEPENDENT_CODE on)

if(WIN32)
  set(PLAT_SOURCES src/windows/dlfcn-win32.cpp
                   src/windows/os-threads.cpp)
endif()

set(platform_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include/platform")
IF(WIN32)
  LIST(APPEND platform_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include/platform/windows")
ENDIF(WIN32)
set(platform_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

if(NOT ${CORE_SYSTEM_NAME} STREQUAL "")
  if(${CORE_SYSTEM_NAME} STREQUAL "darwin" OR ${CORE_SYSTEM_NAME} STREQUAL "ios")
    list(APPEND platform_LIBRARIES "-framework CoreVideo -framework IOKit")
  endif()
endif()

set(SOURCES src/util/fstrcmp.c
            src/util/StringUtils.cpp)

add_library(platform ${SOURCES} ${PLAT_SOURCES})
target_link_libraries(platform ${platform_LIBRARIES})
set_target_properties(platform PROPERTIES VERSION ${platform_VERSION_MAJOR}.${platform_VERSION_MINOR}.${platform_VERSION_PATCH}
                                              SOVERSION ${platform_VERSION_MAJOR}.0)

install(TARGETS platform DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES src/os.h DESTINATION include/platform)
IF(WIN32)
  INSTALL(FILES src/windows/dlfcn-win32.h
                src/windows/os-socket.h
                src/windows/os-threads.h
                src/windows/os-types.h
          DESTINATION include/platform/windows)
ELSE(WIN32)
  install(FILES src/posix/os-socket.h
                src/posix/os-threads.h
                src/posix/os-types.h
          DESTINATION include/platform/posix)
ENDIF(WIN32)
install(FILES src/sockets/cdevsocket.h
              src/sockets/socket.h
              src/sockets/tcp.h
        DESTINATION include/platform/sockets)
install(FILES src/threads/atomics.h
              src/threads/mutex.h
              src/threads/threads.h
        DESTINATION include/platform/threads)
install(FILES src/util/atomic.h
              src/util/buffer.h
              src/util/StringUtils.h
              src/util/StdString.h
              src/util/timeutils.h
              src/util/util.h
        DESTINATION include/platform/util)

IF(NOT WIN32)
  # Pkgconfig
  include(PkgConfigHandler.cmake)
  configure_pc_file(platform platform.pc.in
                                 platform.pc
                                 ${CMAKE_INSTALL_PREFIX}
                                 ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
                                 ${CMAKE_INSTALL_PREFIX}/include)

  install(FILES ${CMAKE_BINARY_DIR}/platform.pc
          DESTINATION ${CMAKE_INSTALL_LIBDIR_NOARCH}/pkgconfig)
ENDIF(NOT WIN32)

# config mode
configure_file (platform-config.cmake.in
                platform-config.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/platform-config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR_NOARCH}/platform)

