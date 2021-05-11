cmake_minimum_required (VERSION 2.6)

message("* vm/emubx1")

add_library(vm_emubx1
	bx1.cpp
	display.cpp
	keyboard.cpp
	printer.cpp
)
