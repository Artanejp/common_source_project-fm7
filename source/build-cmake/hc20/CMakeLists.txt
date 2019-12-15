# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,HC20, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emuhc20)

set(EXEC_TARGET emuhc20)
set(VM_NAME hc20)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE OFF)
set(FLAG_USE_Z80 ON)

set(VMFILES_BASE
      event.cpp
)
set(VMFILES_LIB
      beep.cpp
      datarec.cpp
      disk.cpp
      hd146818p.cpp
      hd6301.cpp
      i8255.cpp
      io.cpp
      mc6800.cpp
      mc6801.cpp
      memory.cpp
      noise.cpp
      
      tf20.cpp
      upd765a.cpp
      z80sio.cpp
)

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

set(VMFILES_HC20 ${VMFILES_BASE})

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(VMFILES ${VMFILES_HC20})
add_definitions(-D_HC20)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/hc20.qrc)

include(config_commonsource)
