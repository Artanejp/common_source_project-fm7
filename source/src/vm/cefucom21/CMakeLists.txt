cmake_minimum_required (VERSION 3.0)

message("* vm/cefucom21")

add_library(vm_emucefucom21
	cefucom21.cpp
	
	mcu.cpp
	pcu.cpp
	../mc6847.cpp
)
