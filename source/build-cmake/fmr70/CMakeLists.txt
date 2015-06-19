# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FMR70, Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")


project (emufmr70)

set(BUILD_FMR70 ON CACHE BOOL "Build for FM-R70")

include(config_emufmr50)

add_subdirectory(../../src/vm/fmr50 vm/fmr50)
#add_subdirectory(../../src/vm/fmgen vm/fmgen)
add_subdirectory(../../src/qt/fmr50 qt/fmr50)


                           
add_subdirectory(../../src/qt/common qt/common)
