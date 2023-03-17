cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(VM_MZ5500_LIB_SRCS
	../mz1p17.cpp

	display.cpp
	keyboard.cpp
	memory.cpp
	sysport.cpp

	mz5500.cpp
)

add_library(vm_${EXE_NAME}
	${VM_MZ5500_LIB_SRCS}
)
