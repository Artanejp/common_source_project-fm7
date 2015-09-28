# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,EPSON HC 80, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufamilybasic)

set(EXEC_TARGET emufamilybasic)

set(LOCAL_LIBS 	   vm_familybasic
		   vm_vm
		   common_common
		   qt_familybasic
		   qt_gui
                   )

set(VMFILES_BASE

  m6502.cpp
  
  datarec.cpp
  
  event.cpp
)


set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER OFF CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_BASE})
add_definitions(-D_FAMILYBASIC)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/familybasic.qrc)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/familybasic)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/familybasic)

include(config_commonsource)



if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/familybasic vm/familybasic)
add_subdirectory(../../src/qt/familybasic qt/familybasic)
     
add_subdirectory(../../src/qt/common qt/common)
