message("* vm/common_vm")

SET(THIS_LIB_VERSION 1.6.0)

#include(cotire)
set(s_vm_common_vm_srcs
	../315-5124.cpp
	../am9511.cpp
	../and.cpp
	../ay_3_891x.cpp
	../beep.cpp
	../datarec.cpp
	../disk.cpp

	../harddisk.cpp
	../hd146818p.cpp
	../hd44102.cpp
	../hd46505.cpp
	../hd63484.cpp
	../huc6280_base.cpp
#	../i286.cpp
#	../i386.cpp
	../i8080_base.cpp

	../i8155.cpp
	../i8237_base.cpp
	../i8251.cpp
	../i8253.cpp
	../i8255.cpp
	../i8259.cpp
#	../i86.cpp
#	../io.cpp
#	../ld700.cpp
	../ls244.cpp
	../ls393.cpp

	../m6502_base.cpp
	../mb8877.cpp
	../mc6800.cpp
	../mc6801.cpp
	../hd6301.cpp
	../mb8861.cpp
	# MC6809 is temporally.
	../mc6809_base.cpp
	../mc6820.cpp
	../mc6840.cpp
	../mc6844.cpp
	../mc6847_base.cpp
	../mc6850.cpp
	../mcs48_base.cpp
	../msm5205.cpp
	../msm58321_base.cpp
#	../mz1p17.cpp
#	../n2a03.cpp
	../nand.cpp
	../noise.cpp
	../nor.cpp
	../not.cpp
	../or.cpp
	../pc6031.cpp
	../pc80s31k.cpp
	../pcm1bit.cpp
	../prnfile.cpp
	../ptf20.cpp
	../rp5c01.cpp
#	../scsi_cdrom.cpp
#	../scsi_dev_base.cpp
#	../scsi_hdd.cpp
#	../scsi_host.cpp
	../sn76489an.cpp
#	../sy6522.cpp
	../t3444a.cpp
	../tf20.cpp
	../tms3631.cpp
	../tms9918a.cpp
#	../tms9995.cpp
	../upd16434.cpp
	../upd1990a.cpp
	../upd4991a.cpp
	../upd71071.cpp
	../upd7220_base.cpp
	../upd765a.cpp
	../upd7752.cpp
	../upd7801.cpp
# 20170527 UPD7180 will not add to lbCSP_common_vm.
##	../upd7810.cpp
# 20170518/MSX: Use v9938.cpp and define "-D_MSX_VDP_MESS" instead of v99x8.cpp .
	../v9938.cpp
##	../v99x8.cpp
#	../w3100a.cpp
	../ym2151.cpp
	../ym2203.cpp
	../ym2413.cpp
	../z80_base.cpp
	../z80ctc.cpp
	../z80dma.cpp
	../z80pio.cpp
	../z80sio.cpp

#	../libcpu_newdev/i86_base.cpp

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
#	../fm7/dummydevice.cpp
)

#set_directory_properties(PROPERTIES
#  COTIRE_PREFIX_HEADER_IGNORE_PATH ${SRC_BASE_DIR}/vm/mame
#)

add_definitions(-D_LIBRARY_NAME=libCSPcommon_vm.${THIS_LIB_VERSION})

if(WIN32)
include (GenerateExportHeader)
#add_library(CSPcommon_vm SHARED
#   ${s_vm_common_vm_srcs}
#    )
   
#target_link_libraries(CSPcommon_vm PUBLIC
#	  ${CMAKE_CURRENT_SOURCE_DIR}/../../../build-cmake/bin-win32/libCSPosd.dll.a
#	  ${CMAKE_CURRENT_SOURCE_DIR}/../../../build-cmake/bin-win32/libCSPemu_utils.dll.a
#	  Qt5::Core
#	  Qt5::Gui
#	  Qt5::OpenGL
#	  Qt5::Widgets
#	  ${SDL2_LIBRARIES}
#	  )
#set_target_properties(CSPcommon_vm PROPERTIES
#			     SOVERSION ${THIS_LIB_VERSION}
#			     VERSION ${THIS_LIB_VERSION}
#			     LINK_INTERFACE_LIBRARIES ""
#			     )
#generate_export_header(CSPcommon_vm
#       BASE_NAME CSPcommon_vm
#        EXPORT_MACRO_NAME CSPcommon_vm_EXPORT
#        EXPORT_FILE_NAME CSPcommon_vm_Export.h
#        STATIC_DEFINE CSPcommon_vm_BUILT_AS_STATIC
#)
add_library(vm_common_vm
   ${s_vm_common_vm_srcs}
    )
else()

  add_library(CSPcommon_vm SHARED
    ${s_vm_common_vm_srcs}
  )
  set_target_properties(CSPcommon_vm PROPERTIES 
     SOVERSION ${THIS_LIB_VERSION}
     VERSION ${THIS_LIB_VERSION}
  )
#  cotire(CSPcommon_vm)
  INSTALL(TARGETS CSPcommon_vm DESTINATION ${LIBCSP_INSTALL_DIR})
endif()
