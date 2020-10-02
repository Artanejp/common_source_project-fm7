cmake_minimum_required (VERSION 2.6)

message("* vm/emujr100")

set(BASIC_VM_FILES
	../sy6522.cpp
	
	./memory.cpp
	jr100.cpp
)
	   
add_library(vm_emujr100  ${BASIC_VM_FILES})
