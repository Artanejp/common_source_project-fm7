# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,PC8801MA, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupc8801ma)


set(LOCAL_LIBS 	   vm_pc88
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
                   qt_pc8801
		   qt_gui
                  )

set(VMFILES
		   z80.cpp
		   z80dma.cpp
#
		   beep.cpp
		   i8251.cpp
		   i8255.cpp
		   pcm1bit.cpp
		   pc80s31k.cpp
		   upd765a.cpp
		   
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

set(BUILD_PC8001SR ON CACHE BOOL "Build for PC8001SR")
set(BUILD_PC8801MA OFF CACHE BOOL "Build with PC8801MA")
set(USE_OPNA ON CACHE BOOL "Use OPNA sound with PC8801MA")
set(USE_PCG  ON CACHE BOOL "Use PCG8100")
set(PC88_EXTRAM_PAGES  "1" CACHE STRING "Set banks of EXTRAM of PC8001, bank = 32Kbytes")
set(USE_CMT_SOUND ON CACHE BOOL "Sound with Data Recorder.")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL if enabled.")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)

if(BUILD_PC8001SR)
  set(EXEC_TARGET emupc8001sr)
  add_definitions(-D_PC8001SR)
  set(VMFILES ${VMFILES}
              upd1990a.cpp
  )
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc8001mk2sr.qrc)
elseif(BUILD_PC8801MA)
  set(EXEC_TARGET emupc8801ma)
  add_definitions(-D_PC8801MA)
  set(VMFILES ${VMFILES}
              upd1990a.cpp
  )
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc8801ma.qrc)
endif()

add_definitions(-DPC88_EXRAM_BANKS=${PC88_EXTRAM_PAGES})

if(USE_PCG)
  set(VMFILES ${VMFILES}
            i8253.cpp
      )      
  add_definitions(-DSUPPORT_PC88_PCG8100)
endif()

if(USE_OPNA)
  add_definitions(-DSUPPORT_PC88_OPNA)
endif()  
if(USE_CMT_SOUND)
  add_definitions(-DDATAREC_SOUND)
endif()

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc8801)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/pc8801ma)

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/pc8801 vm/pc8801)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/pc8801ma qt/pc8801ma)


                           
add_subdirectory(../../src/qt/common qt/common)
