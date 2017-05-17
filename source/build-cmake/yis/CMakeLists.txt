# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject, YAMAHA YIS, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emuyis)

set(EXEC_TARGET emuyis)
set(VM_NAME yis)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)
set(WITH_DEBUGGER ON)

set(VMFILES_BASE
	msm58321.cpp
	m6502.cpp
	io.cpp
	memory.cpp
	event.cpp
)

set(VMFILES_LIB
	am9511.cpp
	beep.cpp
	mb8877.cpp
	mc6820.cpp
	mc6844.cpp
	mc6850.cpp
	msm5832_base.cpp
	m6502_base.cpp
)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")


include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})

add_definitions(-D_YIS)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/yis.qrc)

include(config_commonsource)
