cmake_minimum_required (VERSION 2.6)

message("* vm/familybasic")

add_library(vm_familybasic
	familybasic.cpp
	memory.cpp
	apu.cpp
	ppu.cpp
)