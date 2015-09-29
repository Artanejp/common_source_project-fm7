cmake_minimum_required (VERSION 2.6)

message("* vm/pc100")

set(VM_pc100_LIB_SRCS
	pc100.cpp
	
	crtc.cpp
	ioctrl.cpp
	kanji.cpp
)

add_library(vm_pc100
	${VM_pc100_LIB_SRCS}
)