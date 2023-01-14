cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(VMFILES_M23
	apu.cpp
	beep.cpp
	display.cpp
	floppy.cpp
	iobus.cpp
	keyboard.cpp
	membus.cpp
	m23.cpp
)

set(VMFILES_M68 ${VMFILES_M23})

if(${EXE_NAME} STREQUAL emum23)
	add_library(vm_emum23
		${VMFILES_M23}
	)
endif()

if(${EXE_NAME} STREQUAL emum68)
	add_library(vm_emum68
		${VMFILES_M68}
	)
endif()
