cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(BASIC_VM_FILES_PC98HA 

	bios.cpp
	calendar.cpp
	floppy.cpp
	keyboard.cpp
	./memory.cpp
	note.cpp

	pc98ha.cpp
)
	   

add_library(vm_${EXE_NAME}  ${BASIC_VM_FILES_PC98HA})
