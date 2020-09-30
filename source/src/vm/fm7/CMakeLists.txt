cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")
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

if(${EXE_NAME} STREQUAL emufm8)
	add_library(vm_emufm8
		${VM_FM7_LIB_SRCS}
		fm_bubblecasette.cpp
		fm8_mainio.cpp
	)
elseif(${EXE_NAME} STREQUAL emufm7)
	add_library(vm_emufm7
		${VM_FM7_LIB_SRCS}
		jcommcard.cpp
	)		
elseif(${EXE_NAME} STREQUAL emufm77)
	add_library(vm_emufm77
		${VM_FM7_LIB_SRCS}
		jcommcard.cpp
		floppy_2HD.cpp
	)
elseif(${EXE_NAME} STREQUAL emufm77l2)
	add_library(vm_emufm77l2
		${VM_FM7_LIB_SRCS}
		jcommcard.cpp
		floppy_2HD.cpp
	)
elseif(${EXE_NAME} STREQUAL emufm77l4)
	add_library(vm_emufm77l4
		${VM_FM7_LIB_SRCS}
		jcommcard.cpp
		floppy_2HD.cpp
	)
elseif(${EXE_NAME} STREQUAL emufm77av)
	add_library(vm_emufm77av
		${VM_FM7_LIB_SRCS}
		jcommcard.cpp
	)
else()
	add_library(vm_${EXE_NAME}
		${VM_FM7_LIB_SRCS}
	)		
endif()

if(NOT USE_DEVICES_SHARED_LIB)
	set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} dummydevice.cpp)
	if(${EXE_NAME} STREQUAL emufm77av)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} mb61vh010.cpp)
	endif()
	if(${EXE_NAME} STREQUAL emufm77av20)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} mb61vh010.cpp)
	endif()
	if(${EXE_NAME} STREQUAL emufm77av20ex)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} mb61vh010.cpp hd6844.cpp)
	endif()
	if(${EXE_NAME} STREQUAL emufm77av40)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} mb61vh010.cpp hd6844.cpp)
	endif()
	if(${EXE_NAME} STREQUAL emufm77av40ex)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} mb61vh010.cpp hd6844.cpp)
	endif()
	if(${EXE_NAME} STREQUAL emufm77av40sx)
		set(VM_FM7_LIB_SRCS ${VM_FM7_LIB_SRCS} mb61vh010.cpp hd6844.cpp)
	endif()
endif()

