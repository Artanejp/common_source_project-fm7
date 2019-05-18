# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Hino Electronics CEFUCOM-21, Qt **")
message("")


project (cefucom21)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")


set(VM_NAME cefucom21)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE OFF)

set(FLAG_USE_Z80 ON)
set(VMFILES
		   mc6847.cpp
		   io.cpp
		   memory.cpp
		   event.cpp
)

set(VMFILES_LIB
	ay_3_891x.cpp
	datarec.cpp
	i8255.cpp
	mc6847_mase.cpp
	not.cpp
	rp5c01.cpp
	z80.cpp
	z80ctc.cpp
	z80pio.cpp
)
set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CEFUCOM21)
set(EXEC_TARGET emucefucom21)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/cefucom21.qrc)

include(config_commonsource)
