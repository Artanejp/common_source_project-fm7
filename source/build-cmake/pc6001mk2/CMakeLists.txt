# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,PC6001 mk2, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emupc6001mk2)
set(BUILD_PC6001MK2 ON CACHE BOOL "Build on PC6001 mk2")

include(config_pc6001)

add_subdirectory(../../src/vm/pc6001 vm/pc6001)
add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/machines/pc6001 qt/pc6001) 
add_subdirectory(../../src/qt/debugger qt/debugger)
                          
add_subdirectory(../../src/qt/common qt/common)
