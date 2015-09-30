cmake_minimum_required (VERSION 2.6)

message("* vm/scv")

add_library(vm_scv
	scv.cpp
	
	io.cpp
	memory.cpp
	sound.cpp
	vdp.cpp
)
