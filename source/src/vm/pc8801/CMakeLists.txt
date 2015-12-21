cmake_minimum_required (VERSION 2.6)

message("* vm/pc8801")

if(BUILD_PC98DO)
add_library(vm_pc8801
	pc88.cpp
#	pc8801.cpp
)
else()
add_library(vm_pc8801
	pc88.cpp
	pc8801.cpp
)
endif()