cmake_minimum_required (VERSION 2.6)

message("* vm/emujx")

set(BASIC_VM_FILES 
	./i86.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	speaker.cpp
	jx.cpp
)

add_library(vm_emujx  ${BASIC_VM_FILES})
