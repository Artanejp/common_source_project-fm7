cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

if(${EXE_NAME} STREQUAL emupc8801ma)
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

#if(BUILD_PC98DO)
#add_library(vm_pc8801
#	pc88.cpp
##	pc8801.cpp
#)
#else()
#add_library(vm_pc8801
#	pc88.cpp
#	pc8801.cpp
#)
#endif()
