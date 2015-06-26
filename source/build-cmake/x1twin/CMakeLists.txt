# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,X1 Twin, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emux1twin)

set(BUILD_X1TWIN ON CACHE BOOL "Build for X1 Twin")
include(config_x1)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common/scaler/sse2)
endif()

add_subdirectory(../../src/vm/x1 vm/x1)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/x1turboz qt/x1turboz)
if(BUILD_X1TWIN)
  add_subdirectory(../../src/vm/pcengine vm/pcengine)
endif()

     
add_subdirectory(../../src/qt/common qt/common)
