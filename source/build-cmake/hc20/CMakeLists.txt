# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,HC20, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuhc20)

set(EXEC_TARGET emuhc20)

set(LOCAL_LIBS 	   vm_hc20
		   vm_vm
		   common_common
#		   common_scaler-generic
		   qt_hc20
		   qt_gui
                   )

set(VMFILES_BASE
  mc6800.cpp
  tf20.cpp
  hd146818p.cpp
  
  z80.cpp
  z80sio.cpp
  upd765a.cpp
  i8255.cpp

  beep.cpp
  disk.cpp

  datarec.cpp
  event.cpp
  io.cpp
  memory.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

set(VMFILES_HC20 ${VMFILES_BASE})


include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_HC20})
add_definitions(-D_HC20)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/hc20.qrc)


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/hc20)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/hc20)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/hc20 vm/hc20)
#add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/hc20 qt/hc20)

     
add_subdirectory(../../src/qt/common qt/common)
