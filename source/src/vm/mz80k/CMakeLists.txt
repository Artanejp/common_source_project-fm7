cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

if("${U_EXE_NAME}" STREQUAL "EMUMZ80A")
add_library(vm_${EXE_NAME}
	../mz1p17.cpp

	./memory.cpp
	./keyboard.cpp
	./mz80aif.cpp
	./printer.cpp
	./mz80k.cpp
)

elseif("${U_EXE_NAME}" STREQUAL "EMUMZ80K")
add_library(vm_${EXE_NAME}
	../mz1p17.cpp

	./memory.cpp
	./keyboard.cpp
	./mz80fio.cpp
	./printer.cpp
	./mz80k.cpp
)
elseif("${U_EXE_NAME}" STREQUAL "EMUMZ1200")
add_library(vm_${EXE_NAME}
	../mz1p17.cpp
	./memory.cpp
	./keyboard.cpp
	./mz80fio.cpp
	./printer.cpp
	./mz80k.cpp
)
else()
## IS Exists?
add_library(vm_${EXE_NAME}
	../mz1p17.cpp
	
	./memory.cpp
	./keyboard.cpp
	./printer.cpp
	./mz80k.cpp
)
endif()
