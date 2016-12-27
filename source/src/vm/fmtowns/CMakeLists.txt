cmake_minimum_required (VERSION 2.6)

message("* vm/fm-towns")

add_library(vm_fmtowns
	bios.cpp
	cmos.cpp
	floppy.cpp
	keyboard.cpp
	memory.cpp
	scsi.cpp
	timer.cpp
	fmr50.cpp
)