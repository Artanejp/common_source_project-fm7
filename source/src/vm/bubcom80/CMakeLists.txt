cmake_minimum_required (VERSION 2.6)

message("* vm/bubcom80")

add_library(vm_bubcom80
    bubblecasette.cpp
	cmt.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	membus.cpp
	rtc.cpp
	bubcom80.cpp
)
