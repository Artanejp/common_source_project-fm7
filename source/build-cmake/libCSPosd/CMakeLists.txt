# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

project (libCSPgui)

set(CMAKE_MODULE_PATH "${libCSPgui_SOURCE_DIR}/../cmake")
include(config_sharedlibs)

add_subdirectory(../../src/qt qt/osd)

