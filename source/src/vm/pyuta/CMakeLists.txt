cmake_minimum_required (VERSION 2.6)

message("* vm/emupyuta")

add_library(vm_emupyuta
	../tms9995.cpp
	./memory.cpp
	
	pyuta.cpp
)
