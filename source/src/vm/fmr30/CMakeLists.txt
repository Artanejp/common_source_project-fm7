cmake_minimum_required (VERSION 2.6)

message("* vm/fmr30")

add_library(vm_fmr30
	fmr30.cpp
	cmos.cpp
	floppy.cpp
	keyboard.cpp
	memory.cpp
	rtc.cpp
	serial.cpp
	system.cpp
#	scsi.cpp
	timer.cpp
)