cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

if("${U_EXE_NAME}" STREQUAL "EMUMZ800")
add_library(vm_${EXE_NAME}
	./cmos.cpp
	./emm.cpp
	./kanji.cpp
	./joystick.cpp
	./keyboard.cpp
	./memory.cpp
	./ramfile.cpp
	./floppy.cpp
	./quickdisk.cpp
	
	./mz700.cpp
)
elseif("${U_EXE_NAME}" STREQUAL "EMUMZ1500")
add_library(vm_${EXE_NAME}
	../mz1p17.cpp

	./cmos.cpp
	./emm.cpp
	./joystick.cpp
	./kanji.cpp
	./keyboard.cpp
	./memory.cpp
	./ramfile.cpp
	./floppy.cpp
	./quickdisk.cpp
	./ramfile.cpp
	./psg.cpp
	
	./mz700.cpp
)
else()
# MZ700
	if("${U_EXE_NAME}" STREQUAL "EMUMZ700PAL")
		target_compile_definitions(emumz700pal PUBLIC _PAL)
	endif()
	add_library(vm_${EXE_NAME}
	./cmos.cpp
	./emm.cpp
	./joystick.cpp
	./kanji.cpp
	./keyboard.cpp
	./memory.cpp
	./ramfile.cpp
	
	./mz700.cpp
	)
endif()
