# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,NEC N5200, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emun5200)

set(EXEC_TARGET emun5200)
set(VM_NAME n5200)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)

set(VMFILES_BASE
		   i8237.cpp
		   event.cpp
)
set(VMFILES_LIB
		   beep.cpp
		   i386.cpp
		   i8237_base.cpp
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   io.cpp
		   noise.cpp
		   upd1990a.cpp
		   upd7220.cpp
		   upd765a.cpp
		   disk.cpp
)
set(FLAG_USE_I386_VARIANTS ON)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_N5200)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/n5200.qrc)

include(config_commonsource)
