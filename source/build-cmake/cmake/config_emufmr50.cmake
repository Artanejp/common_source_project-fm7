# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(VM_NAME fmr50)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE ON)

set(VMFILES
#
#		   mb8877.cpp
		   msm58321.cpp
		   scsi_dev.cpp
		   scsi_host.cpp
		   scsi_hdd.cpp
		   memory.cpp

#		   disk.cpp
		   event.cpp
		   io.cpp
)

set(VMFILES_LIB
		   pcm1bit.cpp
		   mb8877.cpp
		   hd46505.cpp
		   upd71071.cpp

		   i8251.cpp
		   i8253.cpp
		   i8259.cpp
		   msm58321_base.cpp
		   
		   hd63484.cpp
#		   i386.cpp  
		   disk.cpp
		   )
		 
set(BUILD_SHARED_LIBS OFF)

set(BUILD_FMR50_286 OFF CACHE BOOL "Build for FM-R50, i286 version")
set(BUILD_FMR50_386 OFF CACHE BOOL "Build for FM-R50, i386 version")
set(BUILD_FMR50_486 OFF CACHE BOOL "Build for FM-R50, i486 version")
set(BUILD_FMR250 OFF CACHE BOOL "Build for FM-R250,  Pentium version of FMR-50")
set(BUILD_FMR60 OFF CACHE BOOL "Build for FM-R60, i286 version")
set(BUILD_FMR70 OFF CACHE BOOL "Build for FM-R70, i386 version")
set(BUILD_FMR80 OFF CACHE BOOL "Build for FM-R80, i486 version")
set(BUILD_FMR280 OFF CACHE BOOL "Build for FM-R250,  Pentium version of FMR-80")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")


include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(BUILD_FMR50_286)
  set(EXEC_TARGET emufmr50_286)
  add_definitions(-D_FMR50)
  add_definitions(-DHAS_I286)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr50.qrc)
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
elseif(BUILD_FMR50_386)
  set(EXEC_TARGET emufmr50_386)
  add_definitions(-D_FMR50)
  add_definitions(-DHAS_I386)
  set(FLAG_USE_I386_VARIANTS ON)
  set(FLAG_USE_I286 OFF)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr50.qrc)
elseif(BUILD_FMR50_486)
  set(EXEC_TARGET emufmr50_486)
  add_definitions(-D_FMR50)
  add_definitions(-DHAS_I486)
  set(FLAG_USE_I386_VARIANTS ON)
  set(FLAG_USE_I286 OFF)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr50.qrc)
elseif(BUILD_FMR250)
  set(EXEC_TARGET emufmr250)
  add_definitions(-D_FMR50)
  add_definitions(-DHAS_PENTIUM)
  set(FLAG_USE_I386_VARIANTS ON)
  set(FLAG_USE_I286 OFF)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr50.qrc)
elseif(BUILD_FMR60)
  set(EXEC_TARGET emufmr60)
  add_definitions(-D_FMR60)
  add_definitions(-DHAS_I286)
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
  set(VMFILES_LIB ${VMFILES_LIB} hd63484.cpp)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr60.qrc)
elseif(BUILD_FMR70)
  set(EXEC_TARGET emufmr70)
  add_definitions(-D_FMR60)
  add_definitions(-DHAS_I386)
  set(FLAG_USE_I386_VARIANTS ON)
  set(FLAG_USE_I286 OFF)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr60.qrc)
elseif(BUILD_FMR80)
  set(EXEC_TARGET emufmr80)
  add_definitions(-D_FMR60)
  add_definitions(-DHAS_I486)
  set(FLAG_USE_I386_VARIANTS ON)
  set(FLAG_USE_I286 OFF)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr60.qrc)
elseif(BUILD_FMR280)
  set(EXEC_TARGET emufmr280)
  add_definitions(-D_FMR60)
  add_definitions(-DHAS_PENTIUM)
  set(FLAG_USE_I386_VARIANTS ON)
  set(FLAG_USE_I286 OFF)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fmr60.qrc)
endif()

include(config_commonsource)


