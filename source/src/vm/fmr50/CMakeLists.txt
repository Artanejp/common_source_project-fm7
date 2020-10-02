cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

set(FMR50_BASE_FILES
	../scsi_host.cpp
	../msm58321.cpp
	
	bios.cpp
	cmos.cpp
	floppy.cpp
	keyboard.cpp
	memory.cpp
	scsi.cpp
	timer.cpp
	
	fmr50.cpp
)

add_library(vm_${EXE_NAME}
	${FMR50_BASE_FILES}	
)

if("${U_EXE_NAME}" STREQUAL "EMUFMR50_286")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_I286)
elseif("${U_EXE_NAME}" STREQUAL "EMUFMR50_386")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_I386)
elseif("${U_EXE_NAME}" STREQUAL "EMUFMR50_486")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_I486)
elseif("${U_EXE_NAME}" STREQUAL "EMUFMR60")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_I286)
elseif("${U_EXE_NAME}" STREQUAL "EMUFMR70")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_I386)
elseif("${U_EXE_NAME}" STREQUAL "EMUFMR80")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_I486)
elseif("${U_EXE_NAME}" STREQUAL "EMUFMR250")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_PENTIUM)
elseif("${U_EXE_NAME}" STREQUAL "EMUFMR280")
	target_compile_definitions(vm_${EXE_NAME} PRIVATE -DHAS_PENTIUM)
endif()
