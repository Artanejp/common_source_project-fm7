# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure CommonSourceProject,FM16β(i86), Qt **")
message("")



project (emufm16beta_86)
set(CMAKE_MODULE_PATH "${emufm16beta_86_SOURCE_DIR}/../cmake")


set(EXEC_TARGET emufm16beta_86)
set(VM_NAME fm16beta_86)
set(BUILD_FM16BETA_86 ON CACHE BOOL "Build for FM16Beta, i86 version")

include(config_emufm16beta)
include(config_commonsource)
