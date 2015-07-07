cmake_minimum_required (VERSION 2.6)

message("* vm/j3100")

set(VM_J3100_LIB_SRCS
	display.cpp
	dmareg.cpp
	floppy.cpp
	j3100.cpp
	keyboard.cpp
	memory.cpp
	sasi.cpp
	slkeyboard.cpp
	slmemory.cpp
	slsystem.cpp
	system.cpp
)

add_library(vm_j3100
	${VM_J3100_LIB_SRCS}
)