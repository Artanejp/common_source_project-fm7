cmake_minimum_required (VERSION 2.6)

message("* vm/x1")

add_library(vm_x1
	display.cpp
	emm.cpp
	floppy.cpp
#	io.cpp
	iobus.cpp
	joystick.cpp
	keyboard.cpp
	memory.cpp
	mouse.cpp
	psub.cpp
	sub.cpp
	x1.cpp
)