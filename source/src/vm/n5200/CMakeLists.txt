message("* vm/emun5200")

set(VM_N5200_LIB_SRCS
	display.cpp
	floppy.cpp
	keyboard.cpp
	memory.cpp
	system.cpp

	n5200.cpp
)

add_library(vm_emun5200
	${VM_N5200_LIB_SRCS}
)
