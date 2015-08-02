cmake_minimum_required (VERSION 2.6)

message("* vm/z80tvgame")

add_library(vm_z80tvgame
	z80tvgame.cpp
	joystick.cpp
	memory.cpp
)