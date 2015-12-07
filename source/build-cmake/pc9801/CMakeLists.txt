# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,PC9801, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupc9801)

set(BUILD_PC9801 ON CACHE BOOL "Build on PC9801")
include(config_emupc9801)

add_subdirectory(../../src/vm/pc9801 vm/pc9801)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/machines/pc9801 qt/pc9801)
if(BUILD_PC98DO)
  add_subdirectory(../../src/vm/pc8801 vm/pc8801)
endif()
                           
add_subdirectory(../../src/qt/common qt/common)
