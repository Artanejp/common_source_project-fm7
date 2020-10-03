cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

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

add_library(vm_${EXE_NAME}
	${VM_PASOPIA_LIB_SRCS}
)
