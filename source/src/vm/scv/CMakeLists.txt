cmake_minimum_required (VERSION 3.0)

message("* vm/emuscv")

add_library(vm_emuscv
	../upd7801.cpp
	
	io.cpp
	memory.cpp
	sound.cpp
	vdp.cpp
	
	scv.cpp
)
