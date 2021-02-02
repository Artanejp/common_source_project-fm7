cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

set(VMFILES_MP
	../i8080.cpp

	display.cpp
	keyboard.cpp
	membus.cpp
	mp85.cpp
)

if(${EXE_NAME} STREQUAL emump85)
	add_library(vm_emump85
		${VMFILES_MP}
	)
endif()
