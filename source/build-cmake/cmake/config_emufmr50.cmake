# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(LOCAL_LIBS 	   vm_fmr50
		   vm_vm
		   common_common
#		   common_scaler-generic
                   qt_fmr50
		   qt_gui
                  )

set(VMFILES
#
		   pcm1bit.cpp

		   i8251.cpp
		   i8253.cpp
		   i8259.cpp
		   hd46505.cpp
		   msm58321.cpp
		   upd71071.cpp
		   mb8877.cpp
		   
		   disk.cpp
		   event.cpp
		   io.cpp
)


set(BUILD_SHARED_LIBS OFF)

set(BUILD_FMR50_286 OFF CACHE BOOL "Build for FM-R50, i286 version")
set(BUILD_FMR50_386 OFF CACHE BOOL "Build for FM-R50, i386 version")
set(BUILD_FMR50_486 OFF CACHE BOOL "Build for FM-R50, i486 version")
set(BUILD_FMR250 OFF CACHE BOOL "Build for FM-R250,  Pentium version of FMR-50")
set(BUILD_FMR60 OFF CACHE BOOL "Build for FM-R60, i286 version")
set(BUILD_FMR70 OFF CACHE BOOL "Build for FM-R70, i386 version")
set(BUILD_FMR80 OFF CACHE BOOL "Build for FM-R80, i486 version")
set(BUILD_FMR280 OFF CACHE BOOL "Build for FM-R250,  Pentium version of FMR-80")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL if enabled.")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(BUILD_FMR50_286)
  set(EXEC_TARGET emufmr50_286)
  add_definitions(-D_FMR50)
  add_definitions(-DHAS_I286)
  set(VMFILES ${VMFILES} i286.cpp)
elseif(BUILD_FMR50_386)
  set(EXEC_TARGET emufmr50_386)
  add_definitions(-D_FMR50)
  add_definitions(-DHAS_I386)
  set(VMFILES ${VMFILES} i386.cpp)
elseif(BUILD_FMR50_486)
  set(EXEC_TARGET emufmr50_486)
  add_definitions(-D_FMR50)
  add_definitions(-DHAS_I486)
  set(VMFILES ${VMFILES} i386.cpp)
elseif(BUILD_FMR250)
  set(EXEC_TARGET emufmr250)
  add_definitions(-D_FMR50)
  add_definitions(-DHAS_PENTIUM)
  set(VMFILES ${VMFILES} i386.cpp)
elseif(BUILD_FMR60_286)
  set(EXEC_TARGET emufmr60)
  add_definitions(-D_FMR60)
  add_definitions(-DHAS_I286)
  set(VMFILES ${VMFILES} i286.cpp hd63484.cpp)
elseif(BUILD_FMR70)
  set(EXEC_TARGET emufmr70)
  add_definitions(-D_FMR60)
  add_definitions(-DHAS_I386)
  set(VMFILES ${VMFILES} i386.cpp hd63484.cpp)
elseif(BUILD_FMR80)
  set(EXEC_TARGET emufmr80)
  add_definitions(-D_FMR60)
  add_definitions(-DHAS_I486)
  set(VMFILES ${VMFILES} i386.cpp hd63484.cpp)
elseif(BUILD_FMR280)
  set(EXEC_TARGET emufmr280)
  add_definitions(-D_FMR60)
  add_definitions(-DHAS_PENTIUM)
  set(VMFILES ${VMFILES} i386.cpp hd63484.cpp)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmr50)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/fmr50)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()

