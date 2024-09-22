message("* vm/${EXE_NAME}")

set(BASIC_VM_FILES_PC98HA

	#bios.cpp
	calendar.cpp
	floppy.cpp
	../pc9801/keyboard.cpp
	membus.cpp
	note.cpp
	../pc9801/serial.cpp

	pc98ha.cpp
)


add_library(vm_${EXE_NAME}  ${BASIC_VM_FILES_PC98HA})
