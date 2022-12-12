cmake_minimum_required (VERSION 3.0)

message("* vm/ex80bs")

add_library(vm_emuex80bs
	../i8080.cpp
	
	cmt.cpp
	display.cpp
	./memory.cpp
	ex80bs.cpp
)
