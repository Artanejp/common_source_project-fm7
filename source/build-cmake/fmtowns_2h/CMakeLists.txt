# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)


project (emufmtowns_2h)
set(CMAKE_MODULE_PATH "${emufmtowns_2h_SOURCE_DIR}/../cmake")

set(BUILD_FMTOWNS_2H ON CACHE BOOL "Build for FM-Towns 2H")

include(config_fmtowns)
#include(config_commonsource)

