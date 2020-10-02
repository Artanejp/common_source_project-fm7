cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

set(BASIC_VM_FILES
	../i8237.cpp
	
	dipsw.cpp
	display.cpp
	floppy.cpp
	fmsound.cpp
	joystick.cpp
	mouse.cpp
	keyboard.cpp
	dmareg.cpp
	membus.cpp
	
    pc9801.cpp
)

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

if("${U_EXE_NAME}" STREQUAL "EMUPC9801") 
	add_library(vm_${EXE_NAME}
		${BASIC_VM_FILES}
		cmt.cpp
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC9801E")
	add_library(vm_${EXE_NAME}
		${BASIC_VM_FILES}
		cmt.cpp
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC9801U")
	add_library(vm_${EXE_NAME}
		${BASIC_VM_FILES}
		cmt.cpp
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC9801VF")
	add_library(vm_${EXE_NAME}
		${BASIC_VM_FILES}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC9801VM")
	add_library(vm_${EXE_NAME}
		${BASIC_VM_FILES}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC98DO")
	add_library(vm_${EXE_NAME}
		../pc8801/pc88.cpp
		${BASIC_VM_FILES}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC9801VX")
	add_library(vm_${EXE_NAME}
		cpureg.cpp
		sasi.cpp
		sasi_bios.cpp
		egc.cpp
		${BASIC_VM_FILES}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC98XL")
	add_library(vm_${EXE_NAME}
		cpureg.cpp
		sasi.cpp
		sasi_bios.cpp
		egc.cpp
		${BASIC_VM_FILES}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC9801RA")
	add_library(vm_${EXE_NAME}
		cpureg.cpp
		sasi.cpp
		sasi_bios.cpp
		egc.cpp
		${BASIC_VM_FILES}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC98RL")
	add_library(vm_${EXE_NAME}
		cpureg.cpp
		sasi.cpp
		sasi_bios.cpp
		egc.cpp
		${BASIC_VM_FILES}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC98XA")
	add_library(vm_${EXE_NAME}
		cpureg.cpp
		sasi.cpp
		sasi_bios.cpp
		${BASIC_VM_FILES}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUPC98DOPLUS")
	add_library(vm_${EXE_NAME}
		../pc8801/pc88.cpp
		cpureg.cpp
		sasi.cpp
		sasi_bios.cpp
		${BASIC_VM_FILES}
	)
endif()
