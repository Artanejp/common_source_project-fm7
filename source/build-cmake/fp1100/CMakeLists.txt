# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FP1100, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufp1100)


set(LOCAL_LIBS 	   qt_osd
                   qt_fp1100
		   qt_gui
		   vm_fp1100
		   vm_vm
		   common_common
		   )

set(VMFILES
		   z80.cpp
		   upd7801.cpp

#		   pcm1bit.cpp
		   beep.cpp
		   upd765a.cpp
		   hd46505.cpp
		   
		   ym2203.cpp
		   
		   datarec.cpp
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)


set(USE_CMT_SOUND ON CACHE BOOL "Sound with Data Recorder.")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

set(WITH_DEBUGGER ON CACHE BOOL "Build with FP1100 debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

  set(EXEC_TARGET emufp1100)
  add_definitions(-D_FP1100)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fp1100.qrc)

if(USE_CMT_SOUND)
  add_definitions(-DDATAREC_SOUND)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fp1100)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/fp1100)

include(config_commonsource)

add_subdirectory(../../src/vm/fp1100 vm/fp1100)
add_subdirectory(../../src/qt/machines/fp1100 qt/fp1100)

                           
add_subdirectory(../../src/qt/common qt/common)
