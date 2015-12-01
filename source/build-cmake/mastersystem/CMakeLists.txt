# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,Sega Master System, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emumastersystem)


set(BUILD_MASTERSYSTEM ON CACHE BOOL "Build for Sega MASTER SYSTEM")

include(config_emumastersystem)

add_subdirectory(../../src/vm/gamegear vm/gamegear)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/machines/gamegear qt/gamegear)
                           
add_subdirectory(../../src/qt/common qt/common)
