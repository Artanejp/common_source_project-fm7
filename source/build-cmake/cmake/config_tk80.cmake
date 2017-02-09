# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,TK-80/80 BS/85, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emutk80bs)

set(WITH_MOUSE ON)

set(VMFILES_BASE
		   i8080.cpp
		   i8255.cpp
		   memory.cpp
		   event.cpp
)
set(VMFILES_LIB
		   pcm1bit.cpp
)

if(BUILD_TK80BS)
    add_definitions(-D_TK80BS)
    set(EXEC_TARGET emutk80bs)
    set(VM_NAME tk80bs)
    set(VMFILES_LIB ${VMFILES_LIB} i8251.cpp)
    set(VMFILES_BASE ${VMFILES_BASE} io.cpp)
    set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/tk80bs.qrc)
elseif(BUILD_TK80)
    add_definitions(-D_TK80)
    set(EXEC_TARGET emutk80)
    set(VM_NAME tk80)
    set(VMFILES_LIB ${VMFILES_LIB} datarec.cpp)
    #set(VMFILES_BASE ${VMFILES_BASE} io.cpp)
    set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/tk80.qrc)
elseif(BUILD_TK85)
    add_definitions(-D_TK85)
    set(EXEC_TARGET emutk85)
    set(VM_NAME tk85)
    set(VMFILES_LIB ${VMFILES_LIB} datarec.cpp)
    #set(VMFILES_BASE ${VMFILES_BASE} io.cpp)
    set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/tk85.qrc)
endif()


set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})

include(config_commonsource)
