message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
	display.cpp
	floppy.cpp
	keyboard.cpp
	mfont.cpp
	./memory.cpp
	qc10.cpp
)
