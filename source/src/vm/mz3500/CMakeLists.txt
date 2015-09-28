cmake_minimum_required (VERSION 2.6)

message("* vm/mz3500")

set(BASIC_VM_FILES 
           mz3500.cpp

	   keyboard.cpp
	   main.cpp
	   sub.cpp
	   )
	   

add_library(vm_mz3500  ${BASIC_VM_FILES})
