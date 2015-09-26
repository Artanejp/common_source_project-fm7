cmake_minimum_required (VERSION 2.6)

message("* vm/fm7")

set(VM_FM7_LIB_SRCS
	display.cpp
	vram.cpp
	keyboard.cpp
	joystick.cpp
	sound.cpp
	floppy.cpp
	fm7_mainio.cpp
	fm7_mainmem.cpp
	kanjirom.cpp
	fm7.cpp
)

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

add_library(vm_fm7
	${VM_FM7_LIB_SRCS}
)