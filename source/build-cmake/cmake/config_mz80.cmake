# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
set(VM_NAME mz80k)
set(USE_FMGEN OFF)
set(USE_DEBUGGER ON)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE OFF)

set(VMFILES_BASE
		   i8253.cpp
		   i8255.cpp
		   prnfile.cpp
		   mz1p17.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)

set(VMFILES_LIB
	   beep.cpp
	   datarec.cpp
	   ls393.cpp
	   pcm1bit.cpp
)		   
set(FLAG_USE_Z80 ON)		   

set(BUILD_MZ80A OFF CACHE BOOL "Build EMU-MZ80A")
set(BUILD_MZ80K OFF CACHE BOOL "Build EMU-MZ80A")
set(BUILD_MZ1200 OFF CACHE BOOL "Build EMU-MZ1200")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")

set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_MZ1200)

set(VMFILES ${VMFILES_BASE}
            and.cpp
#	    t3444a.cpp
#	    disk.cpp
	    )
set(VMFILES_LIB ${VMFILES_LIB}
	    t3444a.cpp
	    disk.cpp
)
add_definitions(-D_MZ1200)
set(EXEC_TARGET emumz1200)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz1200.qrc)
set(BUILD_MZ80FIO ON)

elseif(BUILD_MZ80A)

set(VMFILES ${VMFILES_BASE}
            and.cpp
	    )
add_definitions(-D_MZ80A)
set(EXEC_TARGET emumz80A)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz80a.qrc)
set(BUILD_MZ80AIF ON)

else()

set(BUILD_MZ80FIO ON)
set(VMFILES ${VMFILES_BASE})
add_definitions(-D_MZ80K)
set(EXEC_TARGET emumz80k)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz80k.qrc)
set(VMFILES ${VMFILES_BASE}
#	    t3444a.cpp
#	    disk.cpp
	    )
set(VMFILES_LIB ${VMFILES_LIB}
	    t3444a.cpp
	    disk.cpp
)
endif()

if(BUILD_MZ80A)
set(VMFILES ${VMFILES}
#            mb8877.cpp
#	    disk.cpp
	    io.cpp )
#add_definitions(-DSUPPORT_MZ80AIF)
set(VMFILES_LIB ${VMFILES_LIB}
		mb8877.cpp
	    disk.cpp
)
endif()

include(config_commonsource)
