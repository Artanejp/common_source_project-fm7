cmake_minimum_required (VERSION 2.6)

message("* vm/phc25")

set(VM_PHC25_LIB_SRCS
	phc25.cpp
	memory.cpp
	joystick.cpp
	keyboard.cpp
	system.cpp
)


add_library(vm_phc25
	${VM_PHC25_LIB_SRCS}
)