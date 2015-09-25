cmake_minimum_required (VERSION 2.6)

message("* vm/jr100")

set(BASIC_VM_FILES 
           jr100.cpp
	   memory.cpp
	   )
	   

add_library(vm_jr100  ${BASIC_VM_FILES})
