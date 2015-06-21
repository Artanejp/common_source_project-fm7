# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emux1turboz)

set(EXEC_TARGET emux1turboz)

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

set(BUILD_X1 OFF CACHE BOOL "Build for X1")
set(BUILD_X1TURBO OFF CACHE BOOL "Build for X1 Turbo")
set(BUILD_X1TURBOZ OFF CACHE BOOL "Build for X1 TurboZ")
set(BUILD_X1TWIN OFF CACHE BOOL "Build for X1 twin")

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

if(BUILD_X1)
  set(EXEC_TARGET emux1)
  add_definitions(-D_X1)
elseif(BUILD_X1TURBO)
  set(EXEC_TARGET emux1turbo)
  add_definitions(-D_X1TURBO)
  set(VMFILES ${VMFILES} z80dma.cpp)
elseif(BUILD_X1TURBOZ)
  set(EXEC_TARGET emux1turboz)
  add_definitions(-D_X1TURBOZ)
  set(VMFILES ${VMFILES} z80dma.cpp)
elseif(BUILD_X1TWIN)
  set(EXEC_TARGET emux1twin)
  add_definitions(-D_X1TWIN)
  set(LOCAL_LIBS ${LOCAL_LIBS} vm_pcengine)
  set(VMFILES ${VMFILES} huc6280.cpp)
endif()


if(USE_CMT_SOUND)
add_definitions(-DDATAREC_SOUND)
endif()

#add_definitions(-DUSE_TAPE)
#add_definitions(-DUSE_FD1)
#add_definitions(-DUSE_FD1)


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/x1)
if(BUILD_X1TWIN)
 include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pcengine)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/x1turboz)

include(config_commonsource)

