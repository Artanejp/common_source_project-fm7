
cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(VM_NAME z80tvgame)
set(USE_FMGEN OFF)
set(WITH_MOUSE OFF)
set(WITH_JOYSTICK ON)

set(VMFILES_BASE
  z80.cpp
  event.cpp
)

set(BUILD_I8255  OFF CACHE BOOL "Build I8255 version")
set(BUILD_Z80PIO OFF CACHE BOOL "Build Z80 PIO version")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

add_definitions(-D_Z80TVGAME)
if(BUILD_I8255)
  set(EXEC_TARGET emuz80tvgame_i8255)
  set(VMFILES ${VMFILES_BASE} i8255.cpp)
  set(VMFILES_LIB   pcm1bit.cpp)
  add_definitions(-D_USE_I8255)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/z80tvgame_i8255.qrc)
else()
  set(EXEC_TARGET emuz80tvgame_z80pio)
  set(VMFILES ${VMFILES_BASE})
  set(VMFILES_LIB   pcm1bit.cpp z80pio.cpp)
  add_definitions(-D_USE_Z80PIO)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/z80tvgame_z80pio.qrc)
endif()

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

include(config_commonsource)

