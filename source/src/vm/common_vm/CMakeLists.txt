message("* vm/common_vm")

SET(THIS_LIB_VERSION 3.10.0)

#include(cotire)
set(s_vm_common_vm_srcs

	../device.cpp
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
	../i286.cpp
#	../i386.cpp
	../i386_dasm.cpp
	../i8080_base.cpp

	../i8155.cpp
	../i8237_base.cpp
	../i8251.cpp
	../i8253.cpp
	../i8255.cpp
	../i8259.cpp
	../i8279.cpp
	../i86.cpp
	../io.cpp
#	../ld700.cpp
	../ls244.cpp
	../ls393.cpp

	../m6502_base.cpp
	../mb8877.cpp
	../mc6800.cpp
	../mc6801.cpp
	../hd6301.cpp
	../mb8861.cpp
	../mc6809.cpp
	../mc6820.cpp
	../mc6840.cpp
	../mc6843.cpp
	../mc6844.cpp
	../mc6847_base.cpp
	../mc6850.cpp
	../mcs48_base.cpp
	
	../memory.cpp
	../msm5205.cpp
	../msm58321_base.cpp
	../midi_redirector.cpp

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
	../scsi_cdrom.cpp
	../scsi_dev.cpp
	../scsi_hdd.cpp
#	../scsi_host.cpp
	../sio_redirector.cpp
	
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
	../upd7220.cpp
	../upd765a.cpp
	../upd7752.cpp
	../upd7801.cpp

	../upd7810.cpp
	../upd7907.cpp
	../v30_dasm.cpp
# 20170518/MSX: Use v9938.cpp and define "-D_MSX_VDP_MESS" instead of v99x8.cpp .
	../v9938.cpp
##	../v99x8.cpp
#	../w3100a.cpp
	../ym2151.cpp
	../ym2203.cpp
	../ym2413.cpp
	../z80.cpp
	../z80ctc.cpp
	../z80dma.cpp
	../z80pio.cpp
	../z80sio.cpp

	# FM-7/77/AV
	../fm7/hd6844.cpp
	../fm7/mb61vh010.cpp

	../fmtowns/ad7820kr.cpp
	../fmtowns/rf5c68.cpp
	../fmtowns/ym2612.cpp

#	../i286_np21.cpp
#	../np21/i286c/cpumem.cpp
#	../np21/i286c/i286c.cpp
#	../np21/i286c/i286c_0f.cpp
#	../np21/i286c/i286c_8x.cpp
#	../np21/i286c/i286c_ea.cpp
#	../np21/i286c/i286c_f6.cpp
#	../np21/i286c/i286c_fe.cpp
#	../np21/i286c/i286c_mn.cpp
#	../np21/i286c/i286c_rp.cpp
#	../np21/i286c/i286c_sf.cpp
#	../np21/i286c/v30patch.cpp
	
	../i386_np21.cpp
	../np21/i386c/cpucore.cpp
 	../np21/i386c/cpumem.cpp		   
    ../np21/i386c/ia32/cpu.cpp
     ../np21/i386c/ia32/cpu_io.cpp
     ../np21/i386c/ia32/cpu_mem.cpp
     ../np21/i386c/ia32/ctrlxfer.cpp
     ../np21/i386c/ia32/debug.cpp
     ../np21/i386c/ia32/exception.cpp
     ../np21/i386c/ia32/groups.cpp
     ../np21/i386c/ia32/ia32.cpp
     ../np21/i386c/ia32/inst_table.cpp
     ../np21/i386c/ia32/interface.cpp
     ../np21/i386c/ia32/paging.cpp
     ../np21/i386c/ia32/resolve.cpp
     ../np21/i386c/ia32/segments.cpp
     ../np21/i386c/ia32/task.cpp
     ../np21/i386c/ia32/instructions/bin_arith.cpp
     ../np21/i386c/ia32/instructions/bit_byte.cpp
     ../np21/i386c/ia32/instructions/ctrl_trans.cpp
     ../np21/i386c/ia32/instructions/data_trans.cpp
     ../np21/i386c/ia32/instructions/dec_arith.cpp
     ../np21/i386c/ia32/instructions/flag_ctrl.cpp
     ../np21/i386c/ia32/instructions/fpu.cpp
     ../np21/i386c/ia32/instructions/logic_arith.cpp
     ../np21/i386c/ia32/instructions/misc_inst.cpp
     ../np21/i386c/ia32/instructions/seg_reg.cpp
     ../np21/i386c/ia32/instructions/shift_rotate.cpp
     ../np21/i386c/ia32/instructions/string_inst.cpp
     ../np21/i386c/ia32/instructions/system_inst.cpp
     ../np21/i386c/ia32/instructions/fpu/fpdummy.cpp
     ../np21/i386c/ia32/instructions/fpu/fpemul_dosbox.cpp
     ../np21/i386c/ia32/instructions/fpu/fpemul_dosbox2.cpp
     ../np21/i386c/ia32/instructions/fpu/fpemul_softfloat.cpp
     ../np21/i386c/ia32/instructions/fpu/softfloat/softfloat.cpp
		   
    ../np21/i386c/ia32/instructions/mmx/3dnow.cpp
    ../np21/i386c/ia32/instructions/mmx/mmx.cpp
    ../np21/i386c/ia32/instructions/sse/sse.cpp
    ../np21/i386c/ia32/instructions/sse2/sse2.cpp
    ../np21/i386c/ia32/instructions/sse3/sse3.cpp
#	../libcpu_newdev/device.cpp
)

#set_directory_properties(PROPERTIES
#  COTIRE_PREFIX_HEADER_IGNORE_PATH ${SRC_BASE_DIR}/vm/mame
#)

add_definitions(-D__LIBRARY_NAME=\"libCSPcommon_vm.${THIS_LIB_VERSION}\")

if(WIN32)
include (GenerateExportHeader)
add_library(CSPcommon_vm SHARED
   ${s_vm_common_vm_srcs}
    )
   
target_link_libraries(CSPcommon_vm PRIVATE
          CSPosd
	  CSPemu_utils
	  CSPfmgen
#	  ${CMAKE_CURRENT_SOURCE_DIR}/../../../build-cmake/bin-win32/libCSPosd.dll.a
#	  ${CMAKE_CURRENT_SOURCE_DIR}/../../../build-cmake/bin-win32/libCSPemu_utils.dll.a
#	  Qt5::Core
#	  Qt5::Gui
#	  Qt5::OpenGL
#	  Qt5::Widgets
	  ${SDL2_LIBRARIES}
	  ${ADDITIONAL_DLL_LIBRARIES}
	  )
set_target_properties(CSPcommon_vm PROPERTIES
			     SOVERSION ${THIS_LIB_VERSION}
			     VERSION ${THIS_LIB_VERSION}
			     LINK_INTERFACE_LIBRARIES ""
			     )
	set_std(CSPcommon_vm)
generate_export_header(CSPcommon_vm
       BASE_NAME CSPcommon_vm
        EXPORT_MACRO_NAME CSPcommon_vm_EXPORT
        EXPORT_FILE_NAME CSPcommon_vm_Export.h
        STATIC_DEFINE CSPcommon_vm_BUILT_AS_STATIC
)
#add_library(vm_common_vm
#   ${s_vm_common_vm_srcs}
#    )
else()

  add_library(CSPcommon_vm SHARED
    ${s_vm_common_vm_srcs}
  )
  set_target_properties(CSPcommon_vm PROPERTIES 
     SOVERSION ${THIS_LIB_VERSION}
     VERSION ${THIS_LIB_VERSION}
  )
	set_std(CSPcommon_vm)
#  cotire(CSPcommon_vm)
  INSTALL(TARGETS CSPcommon_vm DESTINATION ${LIBCSP_INSTALL_DIR})
endif()

