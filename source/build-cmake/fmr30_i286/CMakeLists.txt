# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FMR 30, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emufmr30)
set(VM_NAME fmr30)
set(USE_FMGEN OFF)
set(VMFILES
#		   i286.cpp	
		   i8237.cpp
		   fmr50/bios.cpp
		   
		   scsi_dev.cpp
		   scsi_host.cpp
		   scsi_hdd.cpp
		   
		   io.cpp

		   event.cpp
)
set(VMFILES_LIB
		   disk.cpp
		   i8251.cpp
		   i8253.cpp
		   i8259.cpp
		   i8237_base.cpp
		   mb8877.cpp
		   noise.cpp
		   pcm1bit.cpp
		   sn76489an.cpp
#		   scsi_dev_base.cpp
#		   scsi_hdd.cpp
)

set(BUILD_FMR30_86 OFF CACHE BOOL "Build for FM-R30, i86 version")
set(BUILD_FMR30_286 ON CACHE BOOL "Build for FM-R30, i286 version")
if(BUILD_FMR30_86)
  set(EXEC_TARGET emufmr30_i86)
  set(FLAG_USE_I86 OFF)
  set(VMFILES ${VMFILES}
	i286.cpp
  )
  add_definitions(-DHAS_I86)
elseif(BUILD_FMR30_286)
  set(EXEC_TARGET emufmr30_i286)
  set(FLAG_USE_I86 OFF)
  set(VMFILES ${VMFILES}
	i286.cpp
  )
  add_definitions(-DHAS_I286)
endif()

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)
add_definitions(-D_FMR30)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr30.qrc)

include(config_commonsource)
