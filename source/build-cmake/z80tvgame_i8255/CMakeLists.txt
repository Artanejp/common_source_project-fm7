# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Z80 TV Game, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuz80tvgame_z80pio)


set(LOCAL_LIBS 	   vm_z80tvgame
		   vm_vm
		   common_common
		   qt_z80tvgame
		   qt_debugger
		   qt_gui
                   )

set(VMFILES_BASE
  z80.cpp
  pcm1bit.cpp

  event.cpp
)


set(BUILD_I8255  ON CACHE BOOL "Build I8255 version")
set(BUILD_Z80PIO OFF CACHE BOOL "Build Z80 PIO version")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER OFF CACHE BOOL "Build with debugger.")

add_definitions(-D_Z80TVGAME)
if(BUILD_I8255)
  set(EXEC_TARGET emuz80tvgame_i8255)
  set(VMFILES ${VMFILES_BASE} i8255.cpp)
  add_definitions(-D_USE_I8255)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/z80tvgame_i8255.qrc)
else()
  set(EXEC_TARGET emuz80tvgame_z80pio)
  set(VMFILES ${VMFILES_BASE} z80pio.cpp)
  add_definitions(-D_USE_Z80PIO)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/z80tvgame_z80pio.qrc)
endif()
  

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/z80tvgame)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/z80tvgame)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/debugger)

include(config_commonsource)


add_subdirectory(../../src/vm/z80tvgame vm/z80tvgame)
add_subdirectory(../../src/qt/z80tvgame qt/z80tvgame)
add_subdirectory(../../src/qt/debugger qt/debugger)
     
add_subdirectory(../../src/qt/common qt/common)
