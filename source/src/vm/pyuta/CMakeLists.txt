cmake_minimum_required (VERSION 3.0)

message("* vm/emupyuta")

add_library(vm_emupyuta
	../tms9995.cpp
	./memory.cpp
	
	pyuta.cpp
)
