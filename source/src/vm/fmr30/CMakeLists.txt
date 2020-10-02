cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

add_library(vm_${EXE_NAME}
	../fmr50/bios.cpp
	../i8237.cpp
	../scsi_host.cpp
	
	cmos.cpp
	floppy.cpp
	keyboard.cpp
	./memory.cpp
	rtc.cpp
	serial.cpp
	system.cpp
	scsi.cpp
	timer.cpp
	
	fmr30.cpp
)
if("${U_EXE_NAME}" STREQUAL "EMUFMR30_I86")
	message("${U_EXE_NAME}")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_I86)
elseif("${U_EXE_NAME}" STREQUAL "EMUFMR30_I286")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_I286)
endif()
