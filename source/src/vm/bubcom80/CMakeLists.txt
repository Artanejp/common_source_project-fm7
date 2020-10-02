cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
    bubblecasette.cpp
	cmt.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	membus.cpp
	rtc.cpp
	bubcom80.cpp
)
