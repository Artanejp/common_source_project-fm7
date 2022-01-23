cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(VM_J3100_LIB_SRCS
#	../i286.cpp
	../i8237.cpp
	../scsi_host.cpp
	
	display.cpp
	dmareg.cpp
	floppy.cpp
	sasi.cpp
	
	j3100.cpp
)

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)
if("${U_EXE_NAME}" STREQUAL "EMUJ3100GT")
	add_library(vm_${EXE_NAME}

		./memory.cpp
		./system.cpp
		./keyboard.cpp
		${VM_J3100_LIB_SRCS}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUJ3100SL")
	add_library(vm_${EXE_NAME}
		
		slmemory.cpp
		slsystem.cpp
		slkeyboard.cpp
		${VM_J3100_LIB_SRCS}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUJ3100SS")
	add_library(vm_${EXE_NAME}

		./memory.cpp
		system.cpp
		keyboard.cpp
		${VM_J3100_LIB_SRCS}
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUJ3100SE")
	add_library(vm_${EXE_NAME}

		./memory.cpp
		system.cpp
		keyboard.cpp
		${VM_J3100_LIB_SRCS}
	)
endif()
