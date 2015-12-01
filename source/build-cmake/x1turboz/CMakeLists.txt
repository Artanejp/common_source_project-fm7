# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,X1 TurboZ, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emux1turboz)

set(BUILD_X1TURBOZ ON CACHE BOOL "Build for X1 TurboZ")
include(config_x1)

add_subdirectory(../../src/vm/x1 vm/x1)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/machines/x1turboz qt/x1turboz)
if(BUILD_X1TWIN)
  add_subdirectory(../../src/vm/pcengine vm/pcengine)
endif()
     
add_subdirectory(../../src/qt/common qt/common)
