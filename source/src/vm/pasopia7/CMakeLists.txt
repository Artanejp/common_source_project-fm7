cmake_minimum_required (VERSION 2.6)

message("* vm/pasopia7")

set(VM_PASOPIA7_LIB_SRCS
	pasopia7.cpp
	
	display.cpp
	floppy.cpp
	iobus.cpp
	iotrap.cpp
	joypac2.cpp
	kanjipac2.cpp
	keyboard.cpp
	memory.cpp
	pac2.cpp
	rampac2.cpp
)

add_library(vm_pasopia7
	${VM_PASOPIA7_LIB_SRCS}
)