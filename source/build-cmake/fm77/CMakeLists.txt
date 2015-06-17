# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emufm7)


set(BUILD_FM77 ON CACHE BOOL "Build for FM77")

include(config_emufm7)

include(config_commonsource)

add_subdirectory(../../src/vm/fm7 vm/fm7)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/fm7 qt/fm7)
                           
add_subdirectory(../../src/qt/common qt/common)

if(USE_SSE2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()
