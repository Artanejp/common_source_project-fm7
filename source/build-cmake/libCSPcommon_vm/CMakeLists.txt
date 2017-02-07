# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (libCSPcommon_vm)
#include(config_commonsource)

include(config_sharedlibs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/libcpu_newdev/libcpu_i386/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
add_subdirectory(../../src/vm/common_vm vm/common_vm)

