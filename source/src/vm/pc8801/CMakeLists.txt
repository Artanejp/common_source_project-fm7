cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

if("${U_EXE_NAME}" STREQUAL "EMUPC8801MA")
	add_library(vm_emupc8801ma
		../scsi_host.cpp
		pc88.cpp
		pc8801.cpp
	)
else()	
	add_library(vm_${EXE_NAME}
		pc88.cpp
		pc8801.cpp
	)
endif()

