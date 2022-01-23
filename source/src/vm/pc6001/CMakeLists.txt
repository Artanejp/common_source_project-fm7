cmake_minimum_required (VERSION 3.0)

message("* vm/pc6001")

set(BASIC_VM_FILES_PC6001
	../mcs48.cpp
	joystick.cpp
	memory.cpp
	sub.cpp
	timer.cpp
	
	pc6001.cpp
)

if(${EXE_NAME} STREQUAL emupc6001)
	add_library(vm_${EXE_NAME}
		../mc6847.cpp
		display.cpp
		${BASIC_VM_FILES_PC6001}
	)
elseif(${EXE_NAME} STREQUAL emupc6601)
	add_library(vm_${EXE_NAME}
		./floppy.cpp
		${BASIC_VM_FILES_PC6001}
	)
elseif(${EXE_NAME} STREQUAL emupc6601sr)
	add_library(vm_${EXE_NAME}
		./floppy.cpp
		${BASIC_VM_FILES_PC6001}
	)
else()
	add_library(vm_${EXE_NAME}
		${BASIC_VM_FILES_PC6001}
	)
endif()

