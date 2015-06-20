cmake_minimum_required (VERSION 2.6)

message("* vm/hc80")

add_library(vm_hc80
	memory.cpp
	hc80.cpp
	io.cpp
)
