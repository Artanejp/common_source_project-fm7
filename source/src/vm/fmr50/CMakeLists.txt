cmake_minimum_required (VERSION 2.6)

message("* vm/fmr50")

add_library(vm_fmr50
	bios.cpp
	cmos.cpp
	floppy.cpp
	keyboard.cpp
	memory.cpp
	scsi.cpp
	timer.cpp
	fmr50.cpp
	
)
