cmake_minimum_required (VERSION 2.6)

message("* vm/pc16pi")

set(BASIC_VM_FILES 
           fm16pi.cpp
	   sub.cpp
	   )
	   

add_library(vm_fm16pi  ${BASIC_VM_FILES})
