cmake_minimum_required (VERSION 2.6)

message("* vm/qc10")

add_library(vm_qc10
	qc10.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	memory.cpp
	mfont.cpp
)
