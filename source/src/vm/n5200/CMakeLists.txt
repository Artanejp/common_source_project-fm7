cmake_minimum_required (VERSION 2.6)

message("* vm/n5200")

set(VM_N5200_LIB_SRCS
	n5200.cpp
	
	display.cpp
	floppy.cpp
	keyboard.cpp
	memory.cpp
	system.cpp
)

add_library(vm_n5200
	${VM_N5200_LIB_SRCS}
)