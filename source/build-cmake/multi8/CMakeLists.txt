# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Mitsubishi Multi 8, Qt **")
message("")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emumulti8)
set(VM_NAME multi8)
set(USE_FMGEN ON)
set(EXEC_TARGET emumulti8)
set(FLAG_USE_Z80 ON)
set(VMFILES_BASE
		   i8253.cpp
		   io.cpp
		   
		   event.cpp
)
set(VMFILES_LIB
		   datarec.cpp
		   hd46505.cpp
		   i8251.cpp
#		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   beep.cpp
		   upd765a.cpp
		   ym2203.cpp
		   disk.cpp
)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_MULTI8)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/multi8.qrc)

include(config_commonsource)
