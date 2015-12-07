# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,TK-80 BS, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emutk80bs)

set(LOCAL_LIBS 	   qt_osd
		   qt_tk80bs
		   qt_gui
		   vm_tk80bs
		   vm_vm
		   common_common		   
                   )

set(VMFILES_BASE
		   i8080.cpp
		   i8251.cpp
		   i8255.cpp
		   
		   event.cpp
		   io.cpp
		   memory.cpp

		   pcm1bit.cpp
)


set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


set(VMFILES ${VMFILES_BASE})
add_definitions(-D_TK80BS)
set(EXEC_TARGET emutk80bs)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/tk80bs.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/tk80bs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/tk80bs)

include(config_commonsource)

add_subdirectory(../../src/vm/tk80bs vm/tk80bs)
add_subdirectory(../../src/qt/machines/tk80bs qt/tk80bs)
    
add_subdirectory(../../src/qt/common qt/common)
