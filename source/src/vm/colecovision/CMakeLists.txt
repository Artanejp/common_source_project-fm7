cmake_minimum_required (VERSION 3.0)

message("* vm/emucolecovision")

add_library(vm_emucolecovision
	./memory.cpp
	keyboard.cpp

	colecovision.cpp
)
