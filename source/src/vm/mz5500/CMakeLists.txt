cmake_minimum_required (VERSION 2.6)

message("* vm/mz5500")

set(VM_MZ5500_LIB_SRCS
	mz5500.cpp
	
	display.cpp
	keyboard.cpp
	memory.cpp
	sysport.cpp
)

add_library(vm_mz5500
	${VM_MZ5500_LIB_SRCS}
)