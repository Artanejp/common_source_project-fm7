cmake_minimum_required (VERSION 2.6)

message("* vm/fm7")

add_library(vm_fm7
	display.cpp
	keyboard.cpp
	sound.cpp
	floppy.cpp
	fm7_mainio.cpp
	fm7_mainmem.cpp
	kanjirom.cpp
	fm7.cpp
	mb61vh010.cpp
)