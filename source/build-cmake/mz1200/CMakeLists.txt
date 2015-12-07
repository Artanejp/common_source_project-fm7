# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,MZ1200, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emumz1200)

set(BUILD_MZ1200 ON CACHE BOOL "Build EMU-MZ1200")
include(config_mz80)

add_subdirectory(../../src/vm/mz80k vm/mz80)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/machines/mz80k qt/mz80)

     
add_subdirectory(../../src/qt/common qt/common)
