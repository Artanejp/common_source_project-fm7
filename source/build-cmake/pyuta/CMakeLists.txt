# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject, TOMY Pyuta, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupyuta)

set(LOCAL_LIBS 	   qt_osd
		   qt_pyuta
		   qt_gui
		   vm_pyuta
		   vm_vm
		   common_common
                   )

set(VMFILES_BASE
		   tms9995.cpp
		   sn76489an.cpp
		   tms9918a.cpp
		   
		   datarec.cpp
		   
		   event.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
#set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


set(VMFILES ${VMFILES_BASE})
add_definitions(-D_PYUTA)
set(EXEC_TARGET emupyuta)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pyuta.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pyuta)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/pyuta)

include(config_commonsource)



add_subdirectory(../../src/vm/pyuta vm/pyuta)
add_subdirectory(../../src/qt/machines/pyuta qt/pyuta)
     
add_subdirectory(../../src/qt/common qt/common)
