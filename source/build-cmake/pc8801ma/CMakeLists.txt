# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,PC8801MA, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupc8801ma)

set(EXEC_TARGET emupc8801ma)

set(LOCAL_LIBS 	   vm_pc88
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
                   qt_pc8801
		   qt_gui
                  )

set(VMFILES
		   z80.cpp
#		   mcs48.cpp
#
		   z80dma.cpp
#
		   beep.cpp
		   i8251.cpp
		   i8255.cpp
		   pcm1bit.cpp
		   pc80s31k.cpp
		   upd1990a.cpp
		   upd765a.cpp
		   
		   i8253.cpp
		   ym2203.cpp
		   
		   z80ctc.cpp
		   z80pio.cpp
		   z80sio.cpp
#  
		   datarec.cpp
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)

set(WITH_OPNA ON CACHE BOOL "Build with OPNA(YM2608)")
set(WITH_PCG8100 ON CACHE BOOL "Build with PCG8100")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL if enabled.")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

#set(WITH_DEBUGGER ON CACHE BOOL "Build witn XM7 Debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


add_definitions(-D_PC8801MA)

if(WITH_OPNA)
   add_definitions(-DSUPPORT_PC88_OPNA)
endif()
if(WITH_PCG8100)
  add_definitions(-DSUPPORT_PC88_PCG8100)
  set(VMFILES i8253.cpp ${VMFILES})
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc8801)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/pc8801ma)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/pc8801 vm/pc8801)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/pc8801ma qt/pc8801ma)


                           
add_subdirectory(../../src/qt/common qt/common)
