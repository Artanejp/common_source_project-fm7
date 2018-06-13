cmake_minimum_required (VERSION 2.6)

message("* vm/fm7")
#include(cotire)

set(VM_FM7_LIB_SRCS
	display.cpp
	vram.cpp
	keyboard.cpp
	joystick.cpp
	sound.cpp
	floppy.cpp
	fm7_mainio.cpp
	fm7_mainmem.cpp
	mainmem_utils.cpp
	mainmem_writeseq.cpp
	mainmem_readseq.cpp
	mainmem_page2.cpp
	mainmem_mmr.cpp
	kanjirom.cpp
#	dummydevice.cpp
	fm7.cpp
)

if(BUILD_FM8)
  set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} fm_bubblecasette.cpp fm8_mainio.cpp)
elseif(BUILD_FM7)
  set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} jcommcard.cpp)
elseif(BUILD_FMNEW7)
  set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} jcommcard.cpp)
elseif(BUILD_FM77)
  set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} jcommcard.cpp 	floppy_2HD.cpp)
elseif(BUILD_FM77L2)
  set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} jcommcard.cpp 	floppy_2HD.cpp)
elseif(BUILD_FM77L4)
  set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} jcommcard.cpp 	floppy_2HD.cpp)
elseif(BUILD_FM77AV)
  set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} jcommcard.cpp)
endif()

if(USE_DEVICES_SHARED_LIB)
else()
	set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} dummydevice.cpp)
	if(FM77AV_VARIANTS)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} mb61vh010.cpp)
	endif()
	  
	if(BUILD_FM77AV20EX)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} 	hd6844.cpp)
	elseif(BUILD_FM77AV40)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} 	hd6844.cpp)
	elseif(BUILD_FM77AV40EX)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} 	hd6844.cpp)
	elseif(BUILD_FM77AV40SX)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} 	hd6844.cpp)
	endif()
	set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} dummydevice.cpp)
endif()
add_library(vm_fm7
	${VM_FM7_LIB_SRCS}
)
#cotire(vm_fm7)