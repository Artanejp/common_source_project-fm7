cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

set(BASIC_VM_FILES 

	../mz1p17.cpp
	../scsi_host.cpp
	
	crtc.cpp
	floppy.cpp
	joystick.cpp
	keyboard.cpp
	./memory.cpp
	mouse.cpp
	reset.cpp
	sasi.cpp
	serial.cpp
	sysport.cpp
	printer.cpp
	
	mz2800.cpp
)
	   

add_library(vm_emumz2800  ${BASIC_VM_FILES})
