# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,PC6001, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupc6001)


set(LOCAL_LIBS 	   vm_pc60
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
                   qt_pc6001
		   qt_gui
                  )

set(VMFILES
		   z80.cpp
		   i8255.cpp
		   pc6031.cpp
		   pc80s31k.cpp
		   
		   upd765a.cpp
		   ym2203.cpp
		   mcs48.cpp
		   
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
		   datarec.cpp
)


set(BUILD_SHARED_LIBS OFF)

set(BUILD_PC6001 OFF CACHE BOOL "Build on PC6001")
set(BUILD_PC6001MK2 OFF CACHE BOOL "Build on PC6001mk2")
set(BUILD_PC6001MK2SR OFF CACHE BOOL "Build on PC6001mk2SR")
set(BUILD_PC6601 OFF CACHE BOOL "Build on PC6601")
set(BUILD_PC6601SR ON CACHE BOOL "Build on PC6601SR")
set(USE_CMT_SOUND ON CACHE BOOL "Sound with CMT")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL if enabled.")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

#set(WITH_DEBUGGER ON CACHE BOOL "Build witn XM7 Debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_PC6001)
   add_definitions(-D_PC6001)
   set(EXEC_TARGET emupc6001)
   set(VMFILES ${VMFILES}
       mc6847.cpp
   )
elseif(BUILD_PC6001MK2)
   add_definitions(-D_PC6001MK2)
   set(EXEC_TARGET emupc6001mk2)
   set(VMFILES ${VMFILES}
       upd7752.cpp
   )
elseif(BUILD_PC6001MK2SR)
   add_definitions(-D_PC6001MK2SR)
   set(EXEC_TARGET emupc6001mk2sr)
   set(VMFILES ${VMFILES}
       upd7752.cpp
   )
elseif(BUILD_PC6601)
   add_definitions(-D_PC6601)
   set(EXEC_TARGET emupc6601)
   set(VMFILES ${VMFILES}
       upd7752.cpp
   )
elseif(BUILD_PC6601SR)
   add_definitions(-D_PC6601SR)
   set(EXEC_TARGET emupc6601sr)
   set(VMFILES ${VMFILES}
       upd7752.cpp
   )
endif()
if(USE_CMT_SOUND)
   add_definitions(-DDATAREC_SOUND)
endif()

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc6001)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/pc6001)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/pc6001 vm/pc6001)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/pc6001 qt/pc6001)
                           
add_subdirectory(../../src/qt/common qt/common)
