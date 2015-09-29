cmake_minimum_required (VERSION 2.6)

message("* vm/multi8")

add_library(vm_multi8
	multi8.cpp
	cmt.cpp
	display.cpp
	floppy.cpp
	kanji.cpp
	keyboard.cpp
	memory.cpp
	)