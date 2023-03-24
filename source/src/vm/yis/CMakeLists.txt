cmake_minimum_required (VERSION 3.0)

message("* vm/emuyis")

add_library(vm_emuyis
	../m6502.cpp
	../msm58321.cpp

	calendar.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	mapper.cpp
	membus.cpp
	sound.cpp

	yis.cpp
)
