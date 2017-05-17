cmake_minimum_required (VERSION 2.6)

message("* vm/yis")

add_library(vm_yis
	calendar.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	mapper.cpp
	sound.cpp
	yis.cpp
)