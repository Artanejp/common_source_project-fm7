message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
	../n2a03.cpp
	
	apu.cpp
	memory.cpp
	ppu.cpp
	familybasic.cpp
)
