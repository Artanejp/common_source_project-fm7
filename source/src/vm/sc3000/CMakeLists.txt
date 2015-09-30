cmake_minimum_required (VERSION 2.6)

message("* vm/sc3000")

set(VM_SC3000_LIB_SRCS
	sc3000.cpp
	keyboard.cpp
	memory.cpp
)


add_library(vm_sc3000
	${VM_SC3000_LIB_SRCS}
)