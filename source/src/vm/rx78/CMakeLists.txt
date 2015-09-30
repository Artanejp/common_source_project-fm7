cmake_minimum_required (VERSION 2.6)

message("* vm/rx78")

add_library(vm_rx78
	rx78.cpp
	cmt.cpp
	keyboard.cpp
	memory.cpp
	printer.cpp
	vdp.cpp
)
