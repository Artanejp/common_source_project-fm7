# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,PC6601SR, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emupc6601sr)
set(BUILD_PC6601SR ON CACHE BOOL "Build on PC6601SR")
