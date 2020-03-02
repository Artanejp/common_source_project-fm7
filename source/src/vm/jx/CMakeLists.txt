cmake_minimum_required (VERSION 2.6)

message("* vm/jx")

set(BASIC_VM_FILES 
           jx.cpp
	   display.cpp
	   floppy.cpp
	   keyboard.cpp
	   speaker.cpp
	   ./i86.cpp
	   )

add_library(vm_jx  ${BASIC_VM_FILES})
