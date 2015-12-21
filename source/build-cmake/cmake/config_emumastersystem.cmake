# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)
set(VM_NAME gamegear)
set(USE_FMGEN ON)

set(VMFILES
		   z80.cpp
		   i8255.cpp
		   i8251.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)

set(BUILD_SHARED_LIBS OFF)

set(BUILD_MASTERSYSTEM OFF CACHE BOOL "Build for Sega MASTER SYSTEM")
set(BUILD_GAMEGEAR OFF CACHE BOOL "Build for Sega Game Gear")
set(BUILD_MARK3   OFF CACHE BOOL "Build for Sega MARK3")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build witn debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(BUILD_GAMEGEAR)
  set(EXEC_TARGET emugamegear)
  add_definitions(-D_GAMEGEAR)
  set(VMFILES ${VMFILES}
#		   ym2413.cpp
		   sn76489an.cpp
		   315-5124.cpp
		   upd765a.cpp
		   
		   datarec.cpp
		   disk.cpp
		   )
if(USE_CMT_SOUND)
  add_definitions(-DDATAREC_SOUND)
endif()
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/gamegear.qrc)
elseif(BUILD_MARK3)
  set(EXEC_TARGET emumark3)
  add_definitions(-D_MASTERSYSTEM)
  set(VMFILES ${VMFILES}
		   315-5124.cpp
		   ym2413.cpp
		   sn76489an.cpp
		   )
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/segamark3.qrc)
elseif(BUILD_MASTERSYSTEM)
  set(EXEC_TARGET emumastersystem)
  add_definitions(-D_MASTERSYSTEM)
  set(VMFILES ${VMFILES}
		   315-5124.cpp
		   ym2413.cpp
		   sn76489an.cpp
		   )
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mastersystem.qrc)
endif()

include(config_commonsource)

