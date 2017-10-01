# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Hitachi Basic Master Jr., Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emubmjr)
set(VM_NAME bmjr)
set(EXEC_TARGET emubmjr)
set(USE_FMGEN OFF)

set(VMFILES_BASE
	event.cpp
)
set(VMFILES_LIB
	  noise.cpp
	  datarec.cpp
	  mc6800.cpp
	  mc6820.cpp
)

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_BMJR)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/bmjr.qrc)

include(config_commonsource)

