# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FM7, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufm7)


set(LOCAL_LIBS 	   vm_fm7
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
                   qt_fm7
		   qt_gui
                  )

set(VMFILES
		   mc6809.cpp
#
		   pcm1bit.cpp
		   mb8877.cpp
		   
		   ym2203.cpp
		   
		   datarec.cpp
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)

set(BUILD_FM7 ON CACHE BOOL "Build for FM7")
set(BUILD_FM8 OFF CACHE BOOL "Build for FM8")
set(BUILD_FM77 OFF CACHE BOOL "Build with FM77")
set(BUILD_FM77AV OFF CACHE BOOL "Build with FM77AV")
set(BUILD_FM77AV40 OFF CACHE BOOL "Build with FM77AV40")
set(FM77_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77/FM77AV40, bank = 64Kbytes")

set(USE_CMT_SOUND ON CACHE BOOL "Sound with Data Recorder.")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL if enabled.")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

#set(WITH_DEBUGGER ON CACHE BOOL "Build witn XM7 Debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(BUILD_FM7)
  set(EXEC_TARGET emufm7)
  add_definitions(-D_FM7)
  
elseif(BUILD_FM8)
  set(EXEC_TARGET emufm8)
  add_definitions(-D_FM8)
  
elseif(BUILD_FM77)
  set(EXEC_TARGET emufm77)
  add_definitions(-D_FM77)
  
elseif(BUILD_FM77AV)
  set(EXEC_TARGET emufm77av)
  add_definitions(-D_FM77AV)
  
elseif(BUILD_FM77AV40)
  set(EXEC_TARGET emufm77av40)
  add_definitions(-D_FM77AV40)
endif()

add_definitions(-DFM77_EXRAM_BANKS=${FM77_EXTRAM_PAGES})

if(USE_CMT_SOUND)
  add_definitions(-DDATAREC_SOUND)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/fm7)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/fm7 vm/fm7)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/fm7 qt/fm7)


                           
add_subdirectory(../../src/qt/common qt/common)
