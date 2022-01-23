cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

if("${U_EXE_NAME}" STREQUAL "EMUSMC70")
add_library(vm_${EXE_NAME}
	../msm58321.cpp
	
	./memory.cpp
	smc777.cpp
 )
elseif("${U_EXE_NAME}" STREQUAL "EMUSMC777")
add_library(vm_${EXE_NAME}
	./memory.cpp
	smc777.cpp
 )
endif()
