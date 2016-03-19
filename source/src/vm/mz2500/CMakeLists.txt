cmake_minimum_required (VERSION 2.6)

message("* vm/mz2500")

set(MZ2500_SRCS
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

if(BUILD_MZ2500)

add_library(vm_mz2500 ${MZ2500_SRCS})

elseif(BUILD_MZ80B)

add_library(vm_mz2500 ${MZ80B_SRCS})

elseif(BUILD_MZ2000)
add_library(vm_mz2500 ${MZ80B_SRCS}  mz1m01.cpp)

elseif(BUILD_MZ2200)
add_library(vm_mz2500 ${MZ80B_SRCS}  mz1m01.cpp)

endif()
