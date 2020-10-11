cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

add_library(vm_${EXE_NAME}
		../mc6847.cpp
		
		./memory.cpp

		./tvboy.cpp
	)
