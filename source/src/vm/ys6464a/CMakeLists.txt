cmake_minimum_required (VERSION 2.6)

message("* vm/ys6464a")

add_library(vm_ys6464a
	display.cpp
	keyboard.cpp
	ys6464a.cpp
)