cmake_minimum_required (VERSION 3.0)

message("* vm/ex80")

add_library(vm_emuex80
	../i8080.cpp
	
	cmt.cpp
	display.cpp
	keyboard.cpp
	./memory.cpp
	ex80.cpp
)
