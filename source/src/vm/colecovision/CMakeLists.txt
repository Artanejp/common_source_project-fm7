cmake_minimum_required (VERSION 2.6)

message("* vm/colecovision")

add_library(vm_colecovision
	colecovision.cpp
	
	memory.cpp
	keyboard.cpp
)
