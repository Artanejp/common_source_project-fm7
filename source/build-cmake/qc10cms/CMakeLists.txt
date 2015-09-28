# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FM16π, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuqc10cms)


set(LOCAL_LIBS 	   vm_qc10
		   vm_vm
		   common_common
                   qt_qc10
		   qt_gui
                  )

set(VMFILES
		   z80.cpp
		   #
		   z80sio.cpp

		   i8237.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   hd146818p.cpp
		   
		   upd7220.cpp
		   upd765a.cpp
		   
		   io.cpp
		   
		   pcm1bit.cpp
		   disk.cpp
		   event.cpp
)


set(BUILD_SHARED_LIBS OFF)

#set(FM77_EXTRAM_PAGES  "12" CACHE STRING "Set banks of EXTRAM of FM77/FM77AV40, bank = 64Kbytes")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(BUILD_QC10COLOR ON CACHE BOOL "Build emuqc10_cms")

set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

  add_definitions(-D_QC10)
if(BUILD_QC10COLOR)
  set(EXEC_TARGET emuqc10_cms)
  add_definitions(-D_COLOR_MONITOR)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/qc10cms.qrc)
else()
  set(EXEC_TARGET emuqc10)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/qc10.qrc)
endif()

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/qc10)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/qc10)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/qc10 vm/qc10)
add_subdirectory(../../src/qt/qc10 qt/qc10)


                           
add_subdirectory(../../src/qt/common qt/common)
