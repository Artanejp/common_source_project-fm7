cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

set(VMFILES_TK
	display.cpp
	keyboard.cpp

	membus.cpp
)
if(${EXE_NAME} STREQUAL emutk80bs)
	add_library(vm_emutk80bs
		${VMFILES_TK} cmt.cpp
		tk80bs.cpp
	)
else()
	add_library(vm_${EXE_NAME}
		${VMFILES_TK}
		tk80bs.cpp
	)
endif()
