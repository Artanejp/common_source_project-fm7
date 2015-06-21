cmake_minimum_required (VERSION 2.8)

message("* vm/pce")

if(BUILD_X1TWIN)
add_library(vm_pcengine
           pce.cpp
)
else()
add_library(vm_pce
           pce.cpp
	   pcengine.cpp
)
endif()