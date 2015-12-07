# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")

project (emupasopia_lcd)


set(BUILD_PASOPIA_LCD ON CACHE BOOL "Build for PASOPIA with LCD")

include(config_emupasopia)

include(config_commonsource)

if(BUILD_PASOPIA)
  add_subdirectory(../../src/vm/pasopia vm/pasopia)
elseif(BUILD_PASOPIA_LCD)
  add_subdirectory(../../src/vm/pasopia vm/pasopia)
elseif(BUILD_PASOPIA7)
  add_subdirectory(../../src/vm/pasopia7 vm/pasopia7)
elseif(BUILD_PASOPIA7_LCD)
  add_subdirectory(../../src/vm/pasopia7 vm/pasopia7)
endif()

  add_subdirectory(../../src/qt/machines/pasopia qt/pasopia)
  add_subdirectory(../../src/qt/common qt/common)
  add_subdirectory(../../src/qt/debugger qt/debugger)

