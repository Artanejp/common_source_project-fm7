cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(MZ2500_SRCS
	../mz1p17.cpp
	
	calendar.cpp
	cmt.cpp
	floppy.cpp
	interrupt.cpp
	joystick.cpp
	keyboard.cpp
	memory.cpp
	mouse.cpp
	printer.cpp
	mz2500.cpp
	mz1e26.cpp
	mz1e30.cpp
	mz1r13.cpp
	mz1r37.cpp
	serial.cpp
	timer.cpp
	crtc.cpp
	)

set(MZ80B_SRCS
	../mz1p17.cpp
	
	cmt.cpp
	floppy.cpp
	keyboard.cpp
	memory80b.cpp
	printer.cpp
#	mouse.cpp
	mz80b.cpp
	mz1r12.cpp
	mz1r13.cpp
	serial.cpp
	timer.cpp
	)
string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

if("${U_EXE_NAME}" STREQUAL "EMUMZ2500")
	add_library(vm_${EXE_NAME}
		../scsi_host.cpp
		../w3100a.cpp
		${MZ2500_SRCS}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUMZ80B")
	add_library(vm_${EXE_NAME}
			${MZ80B_SRCS}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUMZ2000")
	add_library(vm_${EXE_NAME}
		../mz700/quickdisk.cpp
		mz1m01.cpp
		${MZ80B_SRCS}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUMZ2200")
	add_library(vm_${EXE_NAME}
		../mz700/quickdisk.cpp
		mz1m01.cpp
		${MZ80B_SRCS}
	)
endif()
