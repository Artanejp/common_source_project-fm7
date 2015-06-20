cmake_minimum_required (VERSION 2.6)

message("* vm/hc40")

add_library(vm_hc40
	memory.cpp
	hc40.cpp
	io.cpp
)
