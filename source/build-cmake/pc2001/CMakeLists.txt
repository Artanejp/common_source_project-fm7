# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,NEC PC-100, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (pc2001)
set(VM_NAME pc2001)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE OFF)
set(WITH_DEBUGGER ON)

set(VMFILES
#		   memory.cpp
		   event.cpp
)
set(VMFILES_LIB
		   datarec.cpp
		   noise.cpp
		   pcm1bit.cpp
		   upd16434.cpp
		   upd1990a.cpp
		   upd7907.cpp
)
set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER OFF CACHE BOOL "Build with debugger")
include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_PC2001)
set(EXEC_TARGET emupc2001)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc2001.qrc)

include(config_commonsource)
