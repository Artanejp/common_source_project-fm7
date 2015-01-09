# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,X1, Agar **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupc8801ma)

set(EXEC_TARGET emupc8801ma)

set(LOCAL_LIBS 	   vm_pc88
		   vm_vm
		   common_common
		   vm_fmgen
		   common_scaler-generic
		   agar_ui
		   agar_pc88ma
		   agar_menu
                   )

set(VMFILES
		   z80.cpp
		   mcs48.cpp
#
		   z80dma.cpp
#
		   beep.cpp
		   i8251.cpp
		   i8255.cpp
		   pcm1bit.cpp
		   pc80s31k.cpp
		   upd1990a.cpp
		   upd765a.cpp
		   
		   i8253.cpp
		   ym2203.cpp
		   
		   z80ctc.cpp
		   z80pio.cpp
		   z80sio.cpp
#  
		   datarec.cpp
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)


set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
#set(WITH_DEBUGGER ON CACHE BOOL "Build witn XM7 Debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


add_definitions(-D_PC8801MA)
#add_definitions(-DUSE_TAPE)
#add_definitions(-DUSE_FD1)
#add_definitions(-DUSE_FD1)

include(config_commonsource)


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc8801)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/pc8801ma)



if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/pc8801 vm/pc8801)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/agar/pc8801ma agar/pc8801ma)

set(BUNDLE_LIBS 
			   ${AGAR_LIBS}
			   ${OPENGL_LIBRARY}
			   ${OPENCL_LIBRARY}
			   ${GETTEXT_LIBRARY}
			   ${OPENMP_LIBRARY}
			   ${SDL_LIBRARY}
			   ${THREADS_LIBRARY}
			   fontconfig
			   freetype
			   ${AGAR_DEPLIBS}
)

                           
add_subdirectory(../../src/agar/common agar/common)



if(LIB_RT_HAS_NANOSLEEP)
  add_target_library(emux1 rt)
endif(LIB_RT_HAS_NANOSLEEP)

