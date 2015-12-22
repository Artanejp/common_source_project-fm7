# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Sharp MZ5500/6500/6550, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (mz6550)
set(BUILD_MZ6550 ON CACHE BOOL "Build emumz6550")

include(config_mz5500)
