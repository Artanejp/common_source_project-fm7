# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,SANYO PHC-20, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (phc20)
set(VM_NAME phc20)
set(USE_FMGEN OFF)
set(FLAG_USE_Z80 ON)
set(VMFILES
		   event.cpp
)
set(VMFILES_LIB
		   mc6847.cpp
		   datarec.cpp
)
set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debigger")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_PHC20)
set(EXEC_TARGET emuphc20)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/phc20.qrc)
include(config_commonsource)
