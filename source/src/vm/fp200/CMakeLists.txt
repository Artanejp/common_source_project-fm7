cmake_minimum_required (VERSION 3.0)

message("* vm/emufp200")

add_library(vm_emufp200
	../i8080.cpp
	
	io.cpp
	fp200.cpp
)
