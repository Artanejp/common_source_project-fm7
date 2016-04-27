# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject, Yuasa Kyouiku System YALKY, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emuyalky)

set(EXEC_TARGET emuyalky)
set(VM_NAME yalky)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)
set(WITH_DEBUGGER ON)

set(VMFILES_BASE
	i8080.cpp
	i8155.cpp
	not.cpp
	datarec.cpp
	memory.cpp

	event.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")


include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})

add_definitions(-D_YALKY)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/yalky.qrc)

include(config_commonsource)
