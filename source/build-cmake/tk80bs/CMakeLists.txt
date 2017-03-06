# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,TK-80 BS, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emutk80bs)

set(BUILD_TK80BS ON  CACHE BOOL "Build for TK-80 BS")
set(BUILD_TK80   OFF CACHE BOOL "Build for TK-80")
set(BUILD_TK85   OFF CACHE BOOL "Build for TK-85")

include(config_tk80)