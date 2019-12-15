# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,SM-B-80TE, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuemb80te)
set(EXEC_TARGET emusmb80te)
set(VM_NAME smb80te)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)

set(FLAG_USE_Z80 ON)
set(VMFILES_BASE
       event.cpp
)

set(VMFILES_LIB
       datarec.cpp
       io.cpp
       not.cpp
	   noise.cpp
       z80pio.cpp
)   

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_SMB80TE)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/smb80te.qrc)

include(config_commonsource)
