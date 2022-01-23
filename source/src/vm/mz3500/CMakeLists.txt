cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(BASIC_VM_FILES
	../mz1p17.cpp
	
	keyboard.cpp
	main.cpp
	sub.cpp
	
    mz3500.cpp
)
	   
add_library(vm_${EXE_NAME}  ${BASIC_VM_FILES})
