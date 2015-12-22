# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,QC-10(Monochrome), Qt **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
project (emuqc10)
set(BUILD_QC10 ON CACHE BOOL "Build emuqc10 (Monochrome)")

include(config_qc10)
