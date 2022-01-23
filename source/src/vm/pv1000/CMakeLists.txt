cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(VM_PV1000_LIB_SRCS
	pv1000.cpp

	joystick.cpp
	psg.cpp
	vdp.cpp
)


add_library(vm_${EXE_NAME}
	${VM_PV1000_LIB_SRCS}
)
