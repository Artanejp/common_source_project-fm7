# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Sega Master System, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emumastersystem)


set(LOCAL_LIBS 	   vm_sms
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
                   qt_sms
		   qt_gui
                  )

set(VMFILES
		   z80.cpp
		   i8255.cpp
		   i8251.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)
set(BUILD_MASTERSYSTEM ON CACHE BOOL "Build for Sega MASTER SYSTEM")

include(config_emumastersystem)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/gamegear vm/gamegear)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/gamegear qt/gamegear)


                           
add_subdirectory(../../src/qt/common qt/common)
