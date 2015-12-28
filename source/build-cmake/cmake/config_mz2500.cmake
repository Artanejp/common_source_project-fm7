# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
set(VM_NAME mz2500)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(VMFILES_2500
		   z80.cpp
		   z80pio.cpp
		   z80sio.cpp
		   
		   datarec.cpp
		   disk.cpp
		   
		   i8255.cpp
		   i8253.cpp
		   io.cpp
 
		   pcm1bit.cpp
		   mb8877.cpp
		   rp5c01.cpp
		   ls393.cpp
		   w3100a.cpp
		   
		   ym2203.cpp
		   
		   event.cpp
		   memory.cpp
)
set(VMFILES_BASE
		   event.cpp
		   memory.cpp
		   io.cpp
		   disk.cpp
		   
		   datarec.cpp
		   i8253.cpp
		   i8255.cpp
		   mb8877.cpp
		   pcm1bit.cpp
		   z80.cpp
		   z80pio.cpp
		   
		   )
set(VMFILES_QD
		   z80sio.cpp
		   mz700/quickdisk.cpp
		   )

set(VMFILES_16BIT
		   i286.cpp
		   i8259.cpp
		   )


set(BUILD_MZ2500 OFF CACHE BOOL "Build EMU-MZ2500")
set(BUILD_MZ2200 OFF CACHE BOOL "Build EMU-MZ2200")
set(BUILD_MZ2000 OFF CACHE BOOL "Build EMU-MZ2000")
set(BUILD_MZ80B OFF CACHE BOOL "Build EMU-MZ80B")
set(USE_CMT_SOUND ON CACHE BOOL "Using sound with CMT")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_MZ2500)

set(VMFILES ${VMFILES_2500})
add_definitions(-D_MZ2500)
set(EXEC_TARGET emumz2500)
set(USE_SOCKET ON)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz2500.qrc)
set(USE_FMGEN ON)

elseif(BUILD_MZ2000)
set(VMFILES ${VMFILES_BASE} ${VMFILES_QD} ${VMFILES_16BIT})
add_definitions(-D_MZ2000)
set(EXEC_TARGET emumz2000)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz2000.qrc)
set(USE_FMGEN OFF)

elseif(BUILD_MZ2200)
set(VMFILES ${VMFILES_BASE} ${VMFILES_QD} ${VMFILES_16BIT})
set(LOCAL_LIBS ${LOCAL_LIBS})
add_definitions(-D_MZ2200)
set(EXEC_TARGET emumz2200)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz2200.qrc)
set(USE_FMGEN OFF)

elseif(BUILD_MZ80B)
set(VMFILES ${VMFILES_BASE})
set(LOCAL_LIBS ${LOCAL_LIBS})
add_definitions(-D_MZ80B)
set(EXEC_TARGET emumz80b)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz80b.qrc)
set(USE_FMGEN OFF)

endif()

if(BUILD_MZ80A)
set(VMFILES ${VMFILES}
            mb8877.cpp
	    disk.cpp
	    io.cpp )
#add_definitions(-DSUPPORT_MZ80AIF)
endif()

if(USE_CMT_SOUND)
add_definitions(-DDATAREC_SOUND)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mz2500)
if(BUILD_MZ2200)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mz700)
elseif(BUILD_MZ2000)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mz700)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/machines/mz2500)

include(config_commonsource)
