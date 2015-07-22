cmake_minimum_required (VERSION 2.6)

message("* vm/phc20")

set(VM_PHC20_LIB_SRCS
	phc20.cpp
	memory.cpp
)


add_library(vm_phc20
	${VM_PHC20_LIB_SRCS}
)