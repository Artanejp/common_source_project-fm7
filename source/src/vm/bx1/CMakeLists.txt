cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
	bx1.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	printer.cpp
)
