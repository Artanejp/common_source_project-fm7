cmake_minimum_required (VERSION 2.6)

message("* vm/emurx78")

add_library(vm_emurx78

	cmt.cpp
	keyboard.cpp
	memory.cpp
	printer.cpp
	vdp.cpp
	
	rx78.cpp
)
