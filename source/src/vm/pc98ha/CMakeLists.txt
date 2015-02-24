cmake_minimum_required (VERSION 2.6)

message("* vm/pc98ha")

set(BASIC_VM_FILES 
           pc98ha.cpp
	   bios.cpp
	   calendar.cpp
	   floppy.cpp
	   keyboard.cpp
	   memory.cpp
	   note.cpp
	   printer.cpp
	   )
	   

add_library(vm_pc98ha  ${BASIC_VM_FILES})
