# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,MZ-1500, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupcengine)

set(LOCAL_LIBS     qt_osd
		   qt_pce
		   qt_gui
		   vm_pce
		   vm_vm
		   common_common
                   )

set(VMFILES_BASE
		   huc6280.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


set(VMFILES ${VMFILES_BASE})
add_definitions(-D_PCENGINE)
set(EXEC_TARGET emupcengine)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pcengine.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pcengine)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/pcengine)

include(config_commonsource)

add_subdirectory(../../src/vm/pcengine vm/pcengine)
add_subdirectory(../../src/qt/machines/pcengine qt/pcengine)
     
add_subdirectory(../../src/qt/common qt/common)
