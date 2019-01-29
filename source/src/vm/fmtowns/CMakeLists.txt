cmake_minimum_required (VERSION 2.6)

message("* vm/fm-towns")

add_library(vm_fmtowns

	ad7820kr.cpp
	rf5c68.cpp
	
	adpcm.cpp
	floppy.cpp
	keyboard.cpp
	scsi.cpp
	timer.cpp

	serialrom.cpp
	sysrom.cpp
	dictionary.cpp
	msdosrom.cpp
	
	fontroms.cpp
	
	towns_crtc.cpp
	towns_memory.cpp
	towns_sprite.cpp
	towns_vram.cpp
)
