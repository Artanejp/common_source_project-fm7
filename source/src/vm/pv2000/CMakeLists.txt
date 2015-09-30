cmake_minimum_required (VERSION 2.6)

message("* vm/pv2000")

set(VM_PV2000_LIB_SRCS
	pv2000.cpp

	keyboard.cpp
	printer.cpp
	cmt.cpp
)


add_library(vm_pv2000
	${VM_PV2000_LIB_SRCS}
)