cmake_minimum_required (VERSION 2.6)

message("* vm/mz2800")

set(BASIC_VM_FILES 
           mz2800.cpp
	   crtc.cpp
	   floppy.cpp
	   joystick.cpp
	   keyboard.cpp
	   memory.cpp
	   mouse.cpp
	   reset.cpp
	   sasi.cpp
	   serial.cpp
	   sysport.cpp
	   printer.cpp
	   )
	   

add_library(vm_mz2800  ${BASIC_VM_FILES})
