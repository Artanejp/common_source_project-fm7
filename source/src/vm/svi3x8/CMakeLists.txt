cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

set(SVI3X8_BASE_SRCS
	./joystick.cpp
	./keyboard.cpp
	./memory_ex.cpp

#	../msx/printer.cpp
#	../msx/slot_cart.cpp
#	../msx/slot_mainrom.cpp

	./msx_ex.cpp
)

if("${U_EXE_NAME}" STREQUAL "EMUSVI3X8")
	add_library(vm_${EXE_NAME}
		${SVI3X8_BASE_SRCS}
	)
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -D_MSX_VDP_MESS)
endif()
