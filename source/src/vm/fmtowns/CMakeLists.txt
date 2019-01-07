cmake_minimum_required (VERSION 2.6)

message("* vm/fm-towns")

add_library(vm_fmtowns
	cmos.cpp
	floppy.cpp
	keyboard.cpp
	scsi.cpp
	timer.cpp
	towns_crtc.cpp
	towns_memory.cpp
	towns_sprite.cpp
	towns_vram.cpp
)