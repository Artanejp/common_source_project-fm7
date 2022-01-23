cmake_minimum_required (VERSION 3.0)

message("* vm/emumulti8")

add_library(vm_emumulti8

	cmt.cpp
	display.cpp
	floppy.cpp
	kanji.cpp
	keyboard.cpp
	./memory.cpp
	
	multi8.cpp
)
