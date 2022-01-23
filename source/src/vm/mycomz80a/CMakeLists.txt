cmake_minimum_required (VERSION 3.0)

message("* vm/emumycomz80a")

set(VM_MYCOMZ80A_LIB_SRCS
	../msm5832.cpp
	
	display.cpp
	keyboard.cpp
	./memory.cpp

	mycomz80a.cpp
)


add_library(vm_emumycomz80a
	${VM_MYCOMZ80A_LIB_SRCS}
)
