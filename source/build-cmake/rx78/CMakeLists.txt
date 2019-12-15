# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,BANDAI RX-78, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (rx78)
set(VM_NAME rx78)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(FLAG_USE_Z80 ON)
set(VMFILES
		   event.cpp
)
set(VMFILES_LIB
		   datarec.cpp
		   io.cpp
		   noise.cpp
		   sn76489an.cpp
)
		 
set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger")
include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_RX78)
set(EXEC_TARGET emurx78)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/rx78.qrc)

include(config_commonsource)
