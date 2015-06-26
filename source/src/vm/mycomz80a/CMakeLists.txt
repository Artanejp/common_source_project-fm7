cmake_minimum_required (VERSION 2.6)

message("* vm/mycomz80a")

set(VM_MYCOMZ80A_LIB_SRCS
	display.cpp
	keyboard.cpp
	memory.cpp

	mycomz80a.cpp
)


add_library(vm_mycomz80a
	${VM_MYCOMZ80A_LIB_SRCS}
)