# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

project (libCSPemu_utils)
set(CMAKE_MODULE_PATH "${libCSPemu_utils_SOURCE_DIR}/../cmake")


include(config_sharedlibs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/gui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
add_subdirectory(../../src/qt/emuutils qt/emuutils)


