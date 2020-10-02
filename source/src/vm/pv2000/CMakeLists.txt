cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

set(VM_PV2000_LIB_SRCS
	pv2000.cpp

	keyboard.cpp
	printer.cpp
	cmt.cpp
)


add_library(vm_${EXE_NAME}
	${VM_PV2000_LIB_SRCS}
)
