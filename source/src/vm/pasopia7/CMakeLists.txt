cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

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

add_library(vm_${EXE_NAME}
	${VM_PASOPIA7_LIB_SRCS}
)
