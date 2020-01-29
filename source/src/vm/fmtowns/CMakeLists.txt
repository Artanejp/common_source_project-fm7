cmake_minimum_required (VERSION 2.6)

message("* vm/fm-towns")

set(VM_FMTOWNS_DEV_SRCS
	ad7820kr.cpp
	rf5c68.cpp
	ym2612.cpp
	
	adpcm.cpp
	cdc.cpp
	floppy.cpp
	keyboard.cpp
	scsi.cpp
	timer.cpp
	towns_dmac.cpp
	
	fontroms.cpp
	joystick.cpp

	msdosrom.cpp
	serialrom.cpp
	towns_dictionary.cpp
	
	towns_cdrom.cpp
	towns_crtc.cpp
	towns_memory.cpp
	towns_sprite.cpp
	towns_sysrom.cpp
	towns_vram.cpp

	fmtowns.cpp
)

if(BUILD_WITH_20PIX)
  set(VM_FMTOWNS_DEV_SRCS ${VM_FMTOWNS_DEV_SRCS} fontrom_20pix.cpp)
endif()

add_library(vm_fmtowns
	${VM_FMTOWNS_DEV_SRCS}
)	
