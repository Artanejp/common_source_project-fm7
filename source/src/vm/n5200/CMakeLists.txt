cmake_minimum_required (VERSION 3.0)

message("* vm/emun5200")

set(VM_N5200_LIB_SRCS
	../i8237.cpp
	
	display.cpp
	floppy.cpp
	keyboard.cpp
	memory.cpp
	system.cpp
	
	n5200.cpp
)

add_library(vm_emun5200
	${VM_N5200_LIB_SRCS}
)
