cmake_minimum_required (VERSION 2.6)

message("* vm/pasopia")

set(VM_PASOPIA_LIB_SRCS
	pasopia.cpp
	
	display.cpp
	floppy.cpp
	joypac2.cpp
	kanjipac2.cpp
	keyboard.cpp
	memory.cpp
	pac2.cpp
	rampac2.cpp
)

add_library(vm_pasopia
	${VM_PASOPIA_LIB_SRCS}
)