cmake_minimum_required (VERSION 2.6)

message("* vm/${EXE_NAME}")

set(VM_PHC20_LIB_SRCS
	../mc6847.cpp
	
	./memory.cpp
	phc20.cpp
)


add_library(vm_${EXE_NAME}
	${VM_PHC20_LIB_SRCS}
)
