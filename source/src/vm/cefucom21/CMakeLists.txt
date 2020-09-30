cmake_minimum_required (VERSION 2.6)

message("* vm/cefucom21")

add_library(vm_emucefucom21
	cefucom21.cpp
	
	mcu.cpp
	pcu.cpp
)
