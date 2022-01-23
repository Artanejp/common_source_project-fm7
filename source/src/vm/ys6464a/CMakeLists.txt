cmake_minimum_required (VERSION 3.0)

message("* vm/ys6464a")

add_library(vm_emuys6464a
	display.cpp
	keyboard.cpp
	ys6464a.cpp
)
