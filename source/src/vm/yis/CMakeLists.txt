message("* vm/${EXE_NAME}")
add_library(vm_emuyis
	../m6502.cpp
	../msm58321.cpp

	calendar.cpp
	display.cpp
	floppy.cpp
	keyboard.cpp
	mapper.cpp
	membus.cpp
	sound.cpp

	yis.cpp
)
