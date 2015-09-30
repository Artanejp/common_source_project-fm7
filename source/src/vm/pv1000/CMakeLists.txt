cmake_minimum_required (VERSION 2.6)

message("* vm/pv1000")

set(VM_PV1000_LIB_SRCS
	pv1000.cpp

	joystick.cpp
	psg.cpp
	vdp.cpp
)


add_library(vm_pv1000
	${VM_PV1000_LIB_SRCS}
)