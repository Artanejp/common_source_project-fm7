cmake_minimum_required (VERSION 2.6)

message("* vm/msx1")

add_library(vm_msx1
	joystick.cpp
	keyboard.cpp
	memory.cpp
	msx1.cpp
)