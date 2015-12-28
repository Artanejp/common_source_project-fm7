# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,MSX1, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emumsx1)
set(EXEC_TARGET emumsx1)
set(VM_NAME msx)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(VMFILES_BASE
  z80.cpp
  tms9918a.cpp

  not.cpp
  i8255.cpp
  
  ym2203.cpp
  pcm1bit.cpp
  
  #  
  datarec.cpp
  event.cpp
  io.cpp
  memory.cpp
)

set(BUILD_PX7 OFF CACHE BOOL "Build for Pioneer PX7")
set(BUILD_MSX1 ON CACHE BOOL "Build for MSX1")
set(BUILD_MSX2 OFF CACHE BOOL "Build for MSX2")
set(USE_CMT_SOUND ON CACHE BOOL "Sound with CMT")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

set(VMFILES_MSX2 ${VMFILES_BASE}
                rp5c01.cpp
		v99x8.cpp
		disk.cpp
)

set(VMFILES_MSX1 ${VMFILES_BASE}
                tms9918a.cpp
		disk.cpp
)

set(VMFILES_PX7 ${VMFILES_BASE}
                tms9918a.cpp
                ld700.cpp
)

if(USE_CMT_SOUND)
       set(VMFILES_MSX ${VMFILES_BASE})
endif()

include(detect_target_cpu)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_PX7)
  set(VMFILES ${VMFILES_PX7})
  add_definitions(-D_PX7)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/px7.qrc)
elseif(BUILD_MSX2)
  set(VMFILES ${VMFILES_MSX2})
  add_definitions(-D_MSX2)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/msx2.qrc)
else()
  set(VMFILES ${VMFILES_MSX1})
  add_definitions(-D_MSX1)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/msx1.qrc)
endif()

add_definitions(-DDATAREC_SOUND)
include(config_commonsource)
