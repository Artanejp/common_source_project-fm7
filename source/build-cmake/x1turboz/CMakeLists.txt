# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,X1, Agar **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emux1turboz)

set(EXEC_TARGET emux1turboz)

set(LOCAL_LIBS 	   vm_x1
		   vm_vm
		   common_common
		   vm_fmgen
		   common_scaler-generic
		   agar_ui
		   agar_x1t
		   agar_menu
                   )

set(VMFILES
		   z80.cpp
		   mcs48.cpp
# X1 Twin
#		   huc6280.cpp
# X1 Turbo
		   z80dma.cpp
#
		   beep.cpp
		   hd46505.cpp
		   i8255.cpp
		   ym2151.cpp
		   ym2203.cpp
		   mb8877.cpp
		   upd1990a.cpp
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


add_definitions(-D_X1TURBOZ)
#add_definitions(-DUSE_TAPE)
#add_definitions(-DUSE_FD1)
#add_definitions(-DUSE_FD1)

include(config_commonsource)


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/x1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/x1turboz)



if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/x1 vm/x1)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/agar/x1turboz agar/x1turboz)

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

