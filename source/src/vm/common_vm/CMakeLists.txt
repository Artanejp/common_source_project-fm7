message("* vm/common_vm")

set(s_vm_common_vm_srcs
	../and.cpp
	../ay_3_891x.cpp
	../beep.cpp
	
	../hd146818p.cpp
	../hd63484.cpp
#	i386 is super class.
	../i386_base.cpp
	../i8155.cpp
	
	../i8251.cpp
	
#	../i8259.cpp
#	../ld700.cpp
	../ls244.cpp
	../ls393.cpp

# MC6809 is temporally.
#	../mc6809.cpp
	../mc6820.cpp
	../mc6840.cpp

	../msm5205.cpp

	../nand.cpp
	../nor.cpp
	../not.cpp
	../or.cpp

#	../pc6031.cpp
#	../pc80s31k.cpp
	../pcm1bit.cpp
#	../prnfile.cpp
#	../ptf20.cpp
#	../rp5c01.cpp
#	../scsi_cdrom.cpp
#	../scsi_dev.cpp
#	../scsi_hdd.cpp
#	../scsi_host.cpp
#	../sn76489an.cpp
#	../sy6522.cpp
#	../t3444a.cpp
	../tf20.cpp
	../tms3631.cpp
#	../tms9918a.cpp
#	../tms9995.cpp
#	../upd16434.cpp
#	../upd1990a.cpp
	../upd4991a.cpp
#	../upd71071.cpp
#	../upd7220.cpp
#	../upd765a.cpp
	../upd7752.cpp
#	../upd7801.cpp
#	../upd7810.cpp
#	../v99x8.cpp
#	../w3100a.cpp
#	../wav_sounder.cpp
	../ym2151.cpp
#	../ym2203.cpp
	../ym2413.cpp
#	../z80.cpp
#	../z80ctc.cpp
#	../z80dma.cpp
	../z80pio.cpp
#	../z80sio.cpp
)

if(WIN32)
add_library(vm_common_vm
    ${s_vm_common_vm_srcs}
    )
else()
  add_library(CSPcommon_vm SHARED
    ${s_vm_common_vm_srcs}
  )
  set_target_properties(CSPcommon_vm PROPERTIES 
     SOVERSION 1.0.0
     VERSION 1.0.0
  )
  INSTALL(TARGETS CSPcommon_vm DESTINATION ${LIBCSP_INSTALL_DIR})
endif()
