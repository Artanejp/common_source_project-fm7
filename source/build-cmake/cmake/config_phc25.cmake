# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(VM_NAME phc25)
set(USE_FMGEN ON)
set(WITH_MOUSE OFF)
set(WITH_JOYSTICK ON)

set(FLAG_USE_Z80 ON)
set(VMFILES
		   mc6847.cpp
		   io.cpp
		   
		   event.cpp
)
set(VMFILES_LIB
		   datarec.cpp
		   mc6847_base.cpp
		   not.cpp
		   ym2203.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")
set(BUILD_PHC25 OFF CACHE BOOL "Build ePHC25")
set(BUILD_MAP1010 OFF CACHE BOOL "Build eMAP1010")
set(WITH_DEBUGGER ON CACHE BOOL "Use debugger")

if(BUILD_PHC25)
  add_definitions(-D_PHC25)
  set(EXEC_TARGET emuphc25)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/phc25.qrc)
elseif(BUILD_MAP1010)
  add_definitions(-D_MAP1010)
  set(EXEC_TARGET emumap1010)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/map1010.qrc)
endif()

include(config_commonsource)
