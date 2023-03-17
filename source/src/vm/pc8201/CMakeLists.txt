cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
	cmt.cpp
	keyboard.cpp
	lcd.cpp
	./memory.cpp
	pc8201.cpp
)
