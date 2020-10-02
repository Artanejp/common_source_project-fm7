cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

set(VM_MZ5500_LIB_SRCS
	../i8237.cpp
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
