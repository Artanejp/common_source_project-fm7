cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

if("${U_EXE_NAME}" STREQUAL "EMUGAMEGEAR")
	add_library(vm_${EXE_NAME}
		keyboard.cpp
		./memory.cpp
		system.cpp
		gamegear.cpp
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUMARK3")
	add_library(vm_${EXE_NAME}
		keyboard.cpp
		./memory.cpp
		system.cpp
		mastersystem.cpp
	)
elseif("${U_EXE_NAME}" STREQUAL "EMUMASTERSYSTEM")
	add_library(vm_${EXE_NAME}
		keyboard.cpp
		./memory.cpp
		system.cpp
		mastersystem.cpp
	)
endif()
