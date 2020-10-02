cmake_minimum_required (VERSION 2.6)

message("* vm/emumz3500")

set(BASIC_VM_FILES 
	keyboard.cpp
	main.cpp
	sub.cpp
	
    mz3500.cpp
)
	   

add_library(vm_mz3500  ${BASIC_VM_FILES})
