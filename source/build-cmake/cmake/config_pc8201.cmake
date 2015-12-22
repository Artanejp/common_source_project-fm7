# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(VM_NAME pc8201)
set(USE_FMGEN OFF)
set(VMFILES
		   i8080.cpp
		   i8155.cpp
		   
		   upd1990a.cpp
		   
		   io.cpp
		   pcm1bit.cpp
		   datarec.cpp
		   
		   event.cpp
)


set(BUILD_SHARED_LIBS OFF)

set(BUILD_PC8201  OFF CACHE BOOL "Build on PC-8201")
set(BUILD_PC8201A OFF CACHE BOOL "Build on PC-8201A")
set(USE_CMT_SOUND ON CACHE BOOL "Sound with CMT")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build witn XM7 Debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_PC8201)
   add_definitions(-D_PC8201)
   set(EXEC_TARGET emupc8201)
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc8201.qrc)
elseif(BUILD_PC8201A)
   add_definitions(-D_PC8201A)
   set(EXEC_TARGET emupc8201a)
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc8201a.qrc)
endif()
if(USE_CMT_SOUND)
   add_definitions(-DDATAREC_SOUND)
endif()
include(config_commonsource)

