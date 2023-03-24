cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(VMFILES_TK
	display.cpp
	keyboard.cpp

	membus.cpp
	tk80bs.cpp
)
if(${EXE_NAME} STREQUAL emutk80bs)
	add_library(vm_emutk80bs
		cmt.cpp
		${VMFILES_TK}
	)
elseif(${EXE_NAME} STREQUAL emutk80)
	add_library(vm_emutk80
		cmt.cpp
		${VMFILES_TK}
	)
else()
	add_library(vm_${EXE_NAME}
		${VMFILES_TK}
	)
endif()
