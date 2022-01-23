cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

set(VM_FM16BETA_LIB_SRCS
	../i8237.cpp
	../msm58321.cpp
	
     cmos.cpp
     keyboard.cpp
     mainbus.cpp
     sub.cpp
	 
     fm16beta.cpp
)

add_library(vm_${EXE_NAME}
	${VM_FM16BETA_LIB_SRCS}
)

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

if("${U_EXE_NAME}" STREQUAL "EMUFM16BETA_186")
	target_compile_definitions(vm_emufm16beta_186 PRIVATE -DHAS_I186)
elseif("${U_EXE_NAME}" STREQUAL "EMUFM16BETA_286")
	target_compile_definitions(vm_emufm16beta_286 PRIVATE -DHAS_I286)
endif()
