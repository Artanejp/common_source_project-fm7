cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
	../i8080.cpp

	cmt.cpp
	keyboard.cpp
	lcd.cpp
	./memory.cpp
	pc8201.cpp
)
