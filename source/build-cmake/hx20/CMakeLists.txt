# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,MSX2, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emumsx2plus)
set(EXEC_TARGET emumsx2plus)
set(VM_NAME msx)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(BUILD_PX7 OFF CACHE BOOL "Build for Pioneer PX7")
set(BUILD_MSX1 OFF CACHE BOOL "Build for MSX1")
set(BUILD_MSX2 OFF CACHE BOOL "Build for MSX2")
set(BUILD_MSX2PLUS OFF CACHE BOOL "Build for MSX2+")
set(BUILD_HX20 ON CACHE BOOL "Build for Toshiba HX-20")
set(BUILD_FSA1 OFF CACHE BOOL "Build for Panasonic FS-A1")
set(BUILD_HBF1XDJ OFF CACHE BOOL "Build for Sony HB-F1XDJ")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(config_msx)
