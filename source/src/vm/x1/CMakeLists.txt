cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

if(${EXE_NAME} STREQUAL emux1twin)
	add_library(vm_${EXE_NAME}
		cz8rb.cpp
		display.cpp
		emm.cpp
		floppy.cpp
		iobus.cpp
		joystick.cpp
		keyboard.cpp
		memory.cpp
		mouse.cpp
		psub.cpp
		sasi.cpp
		sub.cpp
		
		../mcs48.cpp
		../mz1p17.cpp
		../scsi_host.cpp
		
		../pcengine/pce.cpp
		../huc6280.cpp
		x1.cpp
	)
else()
	add_library(vm_${EXE_NAME}
		cz8rb.cpp
		display.cpp
		emm.cpp
		floppy.cpp
		iobus.cpp
		joystick.cpp
		keyboard.cpp
		memory.cpp
		mouse.cpp
		psub.cpp
		sasi.cpp
		sub.cpp
		../mcs48.cpp
		../mz1p17.cpp
		../scsi_host.cpp
		x1.cpp
	)
endif()
