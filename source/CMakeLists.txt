if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.27)
  cmake_minimum_required(VERSION 3.5)
else()
  cmake_minimum_required(VERSION 3.0)
endif()

message("* Note: Recommended default values are placed at source/sample-scripts/ , please use them.")
message("* common/common")
# Build Common Sourcecode Project, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

cmake_policy(SET CMP0011 NEW)
#cmake_policy(SET CMP0043 OLD)
#if(CMAKE_VERSION VERSION_LESS 3.5)
#  cmake_policy(SET CMP0005 OLD)
#endif()
if(CMAKE_VERSION VERSION_GREATER 3.8)
  cmake_policy(SET CMP0069 NEW)
endif()
# Note: Belows are temporally disabled, not implemented older CMake.
#project (CSP
#		DESCRIPTION "Common Source Code Project/Qt")
project (CSP)

#ToDo
#set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")
set(USE_DEVICES_SHARED_LIB ON CACHE BOOL "Make devices as one shared library.")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")


include(config)

include(config_emufm7)
include(config_fm16)
include(config_fmr)
include(config_fmtowns)

include(config_casio)
include(config_msx)
include(config_mz80_700)
include(config_sord)
include(config_pc6001)
include(config_pc8801)
include(config_pc9801)
include(config_x1)

include(config_necmisc)

include(config_toshiba)
include(config_epson)
include(config_sega)

include(config_misccom)

include(config_singleboards)


