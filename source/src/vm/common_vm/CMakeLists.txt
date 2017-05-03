message("* vm/common_vm")

set(s_vm_common_vm_srcs
	../and.cpp
	../ay_3_891x.cpp
	../beep.cpp
	
	../hd146818p.cpp
	../hd63484.cpp
	../i8155.cpp
	
	../i8251.cpp
	
#	../i8259.cpp
#	../ld700.cpp
	../ls244.cpp
	../ls393.cpp
	../noise.cpp
	../m6502_base.cpp
	# MC6809 is temporally.
	
	../mc6809_base.cpp
	../mc6820.cpp
	../mc6840.cpp
	../mc6847_base.cpp
	
	../libcpu_newdev/mcs48_base.cpp
	../libcpu_newdev/i86_base.cpp

	../msm5205.cpp
	../msm58321_base.cpp
	
	../nand.cpp
	../nor.cpp
	../not.cpp
	../or.cpp

	../pc6031.cpp
	../pc80s31k.cpp
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
	../t3444a.cpp
	../tf20.cpp
	../tms3631.cpp
#	../tms9918a.cpp
#	../tms9995.cpp
	../upd16434.cpp
#	../upd1990a.cpp
	../upd4991a.cpp
#	../upd71071.cpp
#	../upd7220.cpp
	../upd765a.cpp
	../upd7752.cpp
#	../upd7801.cpp
#	../upd7810.cpp
#	../v9938_base.cpp
#	../v99x8.cpp
#	../w3100a.cpp
	../ym2151.cpp
#	../ym2203.cpp
	../ym2413.cpp
	../z80_base.cpp
#	../z80ctc.cpp
#	../z80dma.cpp
	../z80pio.cpp
#	../z80sio.cpp

	../disk.cpp
	../mb8877.cpp

	../libcpu_newdev/libcpu_i386/i386_opdef.cpp
	../libcpu_newdev/libcpu_i386/i386_base_ext.cpp
	../libcpu_newdev/libcpu_i386/i386op16.cpp
	../libcpu_newdev/libcpu_i386/i386op32.cpp
	../libcpu_newdev/libcpu_i386/i386ops.cpp
	../libcpu_newdev/libcpu_i386/i486ops.cpp
	../libcpu_newdev/libcpu_i386/pentops.cpp
	../libcpu_newdev/libcpu_i386/x87ops.cpp
	../libcpu_newdev/libcpu_i386/vtlb.cpp

	../libcpu_newdev/i386_base.cpp
	
	# FM-7/77/AV
	../fm7/hd6844.cpp
	../fm7/mb61vh010.cpp
	../fm7/dummydevice.cpp
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
     SOVERSION 1.1.2
     VERSION 1.1.2
  )
  INSTALL(TARGETS CSPcommon_vm DESTINATION ${LIBCSP_INSTALL_DIR})
endif()
