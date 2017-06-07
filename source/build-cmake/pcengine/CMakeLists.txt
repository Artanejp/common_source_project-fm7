# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,MZ-1500, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupcengine)
set(VM_NAME pcengine)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE OFF)

set(VMFILES_BASE
		   huc6280.cpp
		   scsi_host.cpp
		   scsi_dev.cpp
		   scsi_cdrom.cpp
		   
		   event.cpp
		   io.cpp
		   memory.cpp
)
set(VMFILES_LIB
		   huc6280_base.cpp
		   msm5205.cpp
#		   scsi_cdrom.cpp
#		   scsi_dev_base.cpp
)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")


set(VMFILES ${VMFILES_BASE})
add_definitions(-D_PCENGINE)
set(EXEC_TARGET emupcengine)
set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pcengine.qrc)
include(config_commonsource)
