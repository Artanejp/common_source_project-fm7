# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)
set(VM_NAME mz700)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(FLAG_USE_Z80 ON)
set(VMFILES_BASE
		   event.cpp
		   io.cpp
		   memory.cpp
)

set(VMFILES_MZ800 ${VMFILES_BASE}
)


set(VMFILES_MZ1500 ${VMFILES_MZ800}
	   prnfile.cpp
	   mz1p17.cpp
)

set(VMFILES_LIB
	   datarec.cpp
	   i8255.cpp
	   i8253.cpp
		   
	   beep.cpp
	   pcm1bit.cpp
	   and.cpp
)
set(VMFILES_LIB_MZ800 
	   z80sio.cpp
	   mb8877.cpp
	   disk.cpp
	   not.cpp
	   z80pio.cpp
	   sn76489an.cpp
)
	 
set(VMFILES_LIB_MZ1500 ${VMFILES_LIB_MZ800}
	   ym2203.cpp
	   prnfile.cpp
)

set(BUILD_MZ700 OFF CACHE BOOL "Build EMU-MZ800")
set(BUILD_MZ800 OFF CACHE BOOL "Build EMU-MZ800")
set(BUILD_MZ1500 OFF CACHE BOOL "Build EMU-MZ1500")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


if(BUILD_MZ1500)

set(VMFILES ${VMFILES_MZ1500})
add_definitions(-D_MZ1500)
set(EXEC_TARGET emumz1500)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz1500.qrc)
set(VMFILES_LIB ${VMFILES_LIB} ${VMFILES_LIB_MZ1500})
set(USE_FMGEN OFF)
elseif(BUILD_MZ800)

set(VMFILES ${VMFILES_MZ800})
add_definitions(-D_MZ800)
set(EXEC_TARGET emumz800)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz800.qrc)
set(VMFILES_LIB ${VMFILES_LIB} ${VMFILES_LIB_MZ800})
set(USE_FMGEN OFF)
else()

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_MZ700)
set(EXEC_TARGET emumz700)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz700.qrc)
set(USE_FMGEN OFF)
endif()

include(config_commonsource)


