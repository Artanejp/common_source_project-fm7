cmake_minimum_required (VERSION 3.0)

message("* vm/crc80")

add_library(vm_emucrc80
	display.cpp
	membus.cpp
	crc80.cpp
)
