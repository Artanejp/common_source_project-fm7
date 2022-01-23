cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(VM_SC3000_LIB_SRCS
	sc3000.cpp
	keyboard.cpp
	memory.cpp
)


add_library(vm_${EXE_NAME}
	${VM_SC3000_LIB_SRCS}
)
