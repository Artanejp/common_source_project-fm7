message("* vm/${EXE_NAME}")

set(VM_MZ5500_LIB_SRCS
	../mz1p17.cpp

	display.cpp
	keyboard.cpp
	membus.cpp
	sysport.cpp

	mz5500.cpp
)

add_library(vm_${EXE_NAME}
	${VM_MZ5500_LIB_SRCS}
)
