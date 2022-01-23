cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
	z80tvgame.cpp
	joystick.cpp
	memory.cpp
)
