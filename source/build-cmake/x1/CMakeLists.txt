# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,X1, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emux1)

set(EXEC_TARGET emux1)

set(LOCAL_LIBS 	   vm_x1
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
		   qt_x1turboz
		   qt_gui
                   )

set(VMFILES
		   z80.cpp
		   mcs48.cpp
# X1 Twin
#		   huc6280.cpp
# X1 Turbo
		   z80dma.cpp
#
		   beep.cpp
		   hd46505.cpp
		   i8255.cpp
		   ym2151.cpp
		   ym2203.cpp
		   mb8877.cpp
		   upd1990a.cpp
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
set(USE_CMT_SOUND ON CACHE BOOL "Sound with CMT")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


add_definitions(-D_X1)
if(USE_CMT_SOUND)
add_definitions(-DDATAREC_SOUND)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/x1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/x1turboz)


include(config_commonsource)



if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/x1 vm/x1)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/x1turboz qt/x1turboz)

     
add_subdirectory(../../src/qt/common qt/common)
