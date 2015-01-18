# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,MZ80/MZ80K/MZ1200, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emumz1200)

set(LOCAL_LIBS 	   vm_mz80
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
		   qt_mz80
		   qt_gui
                   )

set(VMFILES_BASE
		   z80.cpp
		   beep.cpp
		   
		   i8255.cpp
		   i8253.cpp
		   ls393.cpp
		   pcm1bit.cpp
		   
		   datarec.cpp
		   
		   event.cpp
		   io.cpp
		   memory.cpp
)
   


set(BUILD_MZ80A OFF CACHE BOOL "Build EMU-MZ80A")
set(BUILD_MZ1200 ON CACHE BOOL "Build EMU-MZ1200")
set(USE_CMT_SOUND OFF CACHE BOOL "Using sound with CMT")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
#set(WITH_DEBUGGER ON CACHE BOOL "Build witnDebugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


if(BUILD_MZ1200)

set(VMFILES ${VMFILES_BASE}
            and.cpp
	    )
add_definitions(-D_MZ1200)
set(EXEC_TARGET emumz1200)

elseif(BUILD_MZ80A)

set(VMFILES ${VMFILES_BASE}
            and.cpp
	    )
add_definitions(-D_MZ80A)
set(EXEC_TARGET emumz80A)

else()

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_MZ80K)
set(EXEC_TARGET emumz80k)

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/mz80k)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/mz80k)


include(config_commonsource)



if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/mz80k vm/mz80)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/mz80k qt/mz80)

     
add_subdirectory(../../src/qt/common qt/common)
