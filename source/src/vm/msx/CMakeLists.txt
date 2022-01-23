cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

set(MSX_BASE_SRCS
	joystick.cpp
	kanjirom.cpp
	keyboard.cpp
	memory_ex.cpp
	printer.cpp
	scc.cpp
	sound_cart.cpp
	msx_ex.cpp
  )

if("${U_EXE_NAME}" STREQUAL "EMUHBF1XDJ")
	add_library(vm_${EXE_NAME}
		rtcif.cpp
		${MSX_BASE_SRCS}
	)
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -D_MSX_VDP_MESS)
elseif("${U_EXE_NAME}" STREQUAL "EMUPX7")
	add_library(vm_${EXE_NAME}
		../ld700.cpp
		${MSX_BASE_SRCS}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUMSX2PLUS")
	add_library(vm_${EXE_NAME}
		rtcif.cpp
		${MSX_BASE_SRCS}
	)
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -D_MSX_VDP_MESS)
elseif("${U_EXE_NAME}" STREQUAL "EMUMSX2P")
	add_library(vm_${EXE_NAME}
		rtcif.cpp
		${MSX_BASE_SRCS}
	)
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -D_MSX_VDP_MESS)
elseif("${U_EXE_NAME}" STREQUAL "EMUMSX2")
	add_library(vm_${EXE_NAME}
		rtcif.cpp
		${MSX_BASE_SRCS}
	)
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -D_MSX_VDP_MESS)
elseif("${U_EXE_NAME}" STREQUAL "EMUFSA1")
	add_library(vm_${EXE_NAME}
		rtcif.cpp
		${MSX_BASE_SRCS}
	)
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -D_MSX_VDP_MESS)
elseif("${U_EXE_NAME}" STREQUAL "EMUHX20")
	add_library(vm_${EXE_NAME}
		psg_stereo.cpp
		rtcif.cpp
		${MSX_BASE_SRCS}
	)
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -D_MSX_VDP_MESS)
else()
	# standard MSX1
	add_library(vm_${EXE_NAME}
		${MSX_BASE_SRCS}
	)
endif()
