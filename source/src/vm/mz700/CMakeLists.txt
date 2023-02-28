cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

set(VM_MZ700BASE_LIB_SRCS
	./cmos.cpp
	./emm.cpp
	./floppy.cpp
	./kanji.cpp
	./keyboard.cpp
	./memory.cpp

	./quickdisk.cpp
	./ramfile.cpp

	./mz700.cpp
)

if("${U_EXE_NAME}" STREQUAL "EMUMZ800")
add_library(vm_${EXE_NAME}
	${VM_MZ700BASE_LIB_SRCS}
)
elseif("${U_EXE_NAME}" STREQUAL "EMUMZ1500")
add_library(vm_${EXE_NAME}
	../mz1p17.cpp
	./joystick.cpp
	./psg.cpp
	${VM_MZ700BASE_LIB_SRCS}

)
else()
# MZ700
	if("${U_EXE_NAME}" STREQUAL "EMUMZ700PAL")
		target_compile_definitions(emumz700pal PUBLIC _PAL)
	endif()
	add_library(vm_${EXE_NAME}
		./joystick.cpp
		${VM_MZ700BASE_LIB_SRCS}
	)
endif()
