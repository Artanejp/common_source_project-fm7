# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)


project (emufm77av40ex)

set(CMAKE_MODULE_PATH "${emufm77av40ex_SOURCE_DIR}/../cmake")
set(BUILD_FM77AV40EX ON CACHE BOOL "Build for FM77AV40EX")

include(config_emufm7)
include(config_commonsource)
