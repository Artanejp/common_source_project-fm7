# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,PC6001mk2SR, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupc6001mk2sr)
set(BUILD_PC6001MK2SR ON CACHE BOOL "Build on PC6001mk2SR")

include(config_pc6001)
