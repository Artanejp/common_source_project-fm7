cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(VM_NAME mz5500)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(VMFILES
#		   i286.cpp
		   
		   i8237.cpp
		   i8255.cpp
		   i8259.cpp
		   rp5c01.cpp
		   
		   upd7220.cpp
		   ym2203.cpp
		   z80ctc.cpp
		   z80sio.cpp
		   mz1p17.cpp
		   prnfile.cpp
		   
		   event.cpp
		   io.cpp
)

set(VMFILES_LIB
		   ls393.cpp
		   not.cpp
		   upd765a.cpp
		   disk.cpp
)
set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(BUILD_MZ5500 OFF CACHE BOOL "Build emumz5500")
set(BUILD_MZ6500 OFF CACHE BOOL "Build emumz6500")
set(BUILD_MZ6550 OFF CACHE BOOL "Build emumz6550")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_MZ5500)
  add_definitions(-D_MZ5500)
  set(EXEC_TARGET emumz5500)
  set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz5500.qrc)
elseif(BUILD_MZ6500)
  add_definitions(-D_MZ6500)
  set(EXEC_TARGET emumz6500)
  set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz6500.qrc)
else()
  add_definitions(-D_MZ6550)
  set(EXEC_TARGET emumz6550)
  set(FLAG_USE_I286 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/mz6550.qrc)
endif()

include(config_commonsource)
