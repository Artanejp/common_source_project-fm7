cmake_minimum_required (VERSION 2.6)

message("* vm/emuyis")

add_library(vm_emuyis
	../m6502.cpp
	../msm58321.cpp
	
	calendar.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	mapper.cpp
	sound.cpp
	
	yis.cpp
)
