message("* vm/${EXE_NAME}")

cmake_minimum_required (VERSION 3.0)

add_library(vm_emupc2001
	../upd7907.cpp
	
	./io.cpp
	pc2001.cpp
)
