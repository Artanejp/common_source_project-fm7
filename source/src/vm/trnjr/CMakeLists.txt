cmake_minimum_required (VERSION 3.0)

message("* vm/emutrnjr")

add_library(vm_emutrnjr
	display.cpp
	membus.cpp
	
	trnjr.cpp
)
