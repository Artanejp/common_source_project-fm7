message("* vm/${EXE_NAME}")

set(VM_FMTOWNS_DEV_SRCS
#	ad7820kr.cpp
#	rf5c68.cpp
#	ym2612.cpp

	adpcm.cpp

	cmos.cpp
	crtc.cpp
	dictionary.cpp
	dmac.cpp

	floppy.cpp
	fontroms.cpp

	iccard.cpp

	joystick.cpp
	joypad_2btn.cpp
	joypad_6btn.cpp
	keyboard.cpp

	mouse.cpp
	msdosrom.cpp

	scsi.cpp
	sprite.cpp
	sysrom.cpp
	timer.cpp
	towns_memory.cpp
	towns_serialrom.cpp

	#towns_scsi_host.cpp

	vram.cpp
	planevram.cpp

	cdrom/cdrom.cpp
	cdrom/cdrom_cue_parser.cpp
	cdrom/cdrom_ccd_parser.cpp
	cdrom/cdrom_iso_parser.cpp

	../scsi_host.cpp
	../msm58321.cpp
	./mb87078.cpp
	./serialrom.cpp

	fmtowns.cpp
)

# For Towns2 HRxx , Towns2 MAxxx , Towns2 MXxxx have 20pix FONT ROM,
# extended VRAM and High resolution CRTC.
# For Towns2 MAxx, Towns2 MExxx, Towns2 MXxxx, Towns2 MFxxx and Towns2 Fresh
# have another DMAC , faster speed CD-ROM and new (YAT) PCM AD/DA sound.
# ToDo: Implement High resolution CRTC , faster speed CD-ROM (with cache)
#       and YAT PCM sound.
#       -- 20250121 K.Ohta.

## ToDo: I don't know what have FMV Towns :-)
## -- 20250121 K.Ohta.

if(${EXE_NAME} STREQUAL emufmtowns2HR20)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2HR100)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2HR200)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2ME20)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2ME170)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2MA20)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2MA170)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2MA340)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2MX20)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2MX170)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
elseif(${EXE_NAME} STREQUAL emufmtowns2MX340)
  set(VM_FMTOWNS_DEV_SRCS fontrom_20pix.cpp ${VM_FMTOWNS_DEV_SRCS})
endif()

add_library(vm_${EXE_NAME}
	${VM_FMTOWNS_DEV_SRCS}
)
