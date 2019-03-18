# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(VM_NAME fm16beta)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)

set(VMFILES
		   i8237.cpp
		   msm58321.cpp
#		   scsi_dev.cpp
#		   scsi_host.cpp
#		   scsi_hdd.cpp
		   memory.cpp
		   
		   event.cpp
		   io.cpp
)

set(VMFILES_LIB
		   hd46505.cpp

		   i8237_base.cpp
		   i8251.cpp
		   i8259.cpp
		   msm58321_base.cpp
		   mb8877.cpp
		   mc6840.cpp
		   pcm1bit.cpp

		   mb61vh010.cpp
		   noise.cpp
		   disk.cpp
		   )
set(FLAG_USE_MC6809 ON)
		 
set(BUILD_SHARED_LIBS OFF)

set(BUILD_FM16BETA_286 OFF CACHE BOOL "Build for FM16Beta, i286 version")
set(BUILD_FM16BETA_86 OFF CACHE BOOL "Build for FM16Beta, i86 version")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")


include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(BUILD_FM16BETA_286)
  set(EXEC_TARGET emufm16beta_286)
  add_definitions(-D_FM16BETA)
  add_definitions(-DHAS_I286)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm16beta.qrc)
  set(FLAG_USE_I286 ON)
elseif(BUILD_FM16BETA_86)
  set(EXEC_TARGET emufm16beta_86)
  add_definitions(-D_FM16BETA)
  add_definitions(-DHAS_I86)
  set(FLAG_USE_I286 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fm16beta.qrc)
endif()

#include(config_commonsource)


