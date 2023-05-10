cmake_minimum_required (VERSION 3.0)

message("* vm/fm-towns")

set(VM_FMTOWNS_DEV_SRCS
#	ad7820kr.cpp
#	rf5c68.cpp
#	ym2612.cpp

	adpcm.cpp

	floppy.cpp
	keyboard.cpp
	scsi.cpp
	timer.cpp
	dmac.cpp

	fontroms.cpp
	iccard.cpp
	joystick.cpp
	joypad_2btn.cpp
	joypad_6btn.cpp
	mouse.cpp

	msdosrom.cpp
	serialrom.cpp
	dictionary.cpp

	cdrom/cdrom.cpp
	cdrom/cdrom_cue_parser.cpp
	cdrom/cdrom_ccd_parser.cpp
	cdrom/cdrom_iso_parser.cpp

	crtc.cpp
	towns_memory.cpp
	#towns_scsi_host.cpp
	sprite.cpp
	sysrom.cpp
	vram.cpp
	planevram.cpp
	../scsi_host.cpp
	../msm58321.cpp

	./mb87078.cpp

	fmtowns.cpp
)

#ToDo
if(BUILD_WITH_20PIX)
  set(VM_FMTOWNS_DEV_SRCS ${VM_FMTOWNS_DEV_SRCS} fontrom_20pix.cpp)
endif()

add_library(vm_${EXE_NAME}
	${VM_FMTOWNS_DEV_SRCS}
)
