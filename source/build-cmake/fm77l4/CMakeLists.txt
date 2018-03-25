# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)


project (emufm77l4)

set(CMAKE_MODULE_PATH "${emufm77l4_SOURCE_DIR}/../cmake")
set(BUILD_FM77L4 ON CACHE BOOL "Build for FM77L4")

include(config_emufm7)
include(config_commonsource)
