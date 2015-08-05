# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Toshiba J3100, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuj3100sl)


set(LOCAL_LIBS 	   vm_j3100
		   vm_vm
		   common_common
                   qt_j3100
		   qt_gui
                  )

set(VMFILES
		   hd46505.cpp
		   i8237.cpp
		   i8253.cpp
		   i8259.cpp
		   
		   io.cpp
		   disk.cpp
		   
		   pcm1bit.cpp
		   upd765a.cpp
		   
		   event.cpp
)

set(BUILD_J3100GT ON  CACHE BOOL "Build emuJ3100GT")
set(BUILD_J3100SL OFF CACHE BOOL "Build emuJ3100SL")
set(BUILD_J3100SS OFF CACHE BOOL "Build emuJ3100SS")
set(BUILD_J3100SE OFF CACHE BOOL "Build emuJ3100SE")

set(BUILD_SHARED_LIBS OFF)

#set(FM77_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77/FM77AV40, bank = 64Kbytes")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

set(WITH_DEBUGGER OFF CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(BUILD_J3100GT)
  set(EXEC_TARGET emuj3100gt)
  add_definitions(-D_J3100GT)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/j3100gt.qrc)
  set(VMFILES ${VMFILES} 
               i286.cpp 
	       hd146818p.cpp
	       )
elseif(BUILD_J3100SL)
  set(EXEC_TARGET emuj3100sl)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/j3100sl.qrc)
  add_definitions(-D_J3100SL)
  set(VMFILES ${VMFILES} 
               i86.cpp 
	       rp5c01.cpp
	       )
elseif(BUILD_J3100SS)
  set(EXEC_TARGET emuj3100ss)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/j3100ss.qrc)
  add_definitions(-D_J3100SS)
  set(VMFILES ${VMFILES} 
               i86.cpp 
	       rp5c01.cpp
	       )
elseif(BUILD_J3100SE)
  set(EXEC_TARGET emuj3100se)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/j3100se.qrc)
  add_definitions(-D_J3100SE)
  set(VMFILES ${VMFILES} 
               i86.cpp 
	       rp5c01.cpp
	       )
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/j3100)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/j3100)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/j3100 vm/j3100)
add_subdirectory(../../src/qt/j3100 qt/j3100)
                           
add_subdirectory(../../src/qt/common qt/common)
