message("* vm/${EXE_NAME}")

set(VMFILES_MP
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
