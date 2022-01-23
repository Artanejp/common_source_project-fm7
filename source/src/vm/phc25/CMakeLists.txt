cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(VM_PHC25_LIB_SRCS
	../mc6847.cpp

	./memory.cpp
	joystick.cpp
	keyboard.cpp
	system.cpp
	
	phc25.cpp
)


add_library(vm_${EXE_NAME}
	${VM_PHC25_LIB_SRCS}
)
