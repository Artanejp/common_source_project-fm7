cmake_minimum_required (VERSION 2.6)

message("* vm/emufm16pi")

set(BASIC_VM_FILES
	../msm58321.cpp
	
	sub.cpp
	fm16pi.cpp
)

add_library(vm_emufm16pi  ${BASIC_VM_FILES})
