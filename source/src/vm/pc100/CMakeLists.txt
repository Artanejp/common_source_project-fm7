cmake_minimum_required (VERSION 3.0)

message("* vm/emupc100")

set(VM_emupc100_LIB_SRCS
	../msm58321.cpp
	
	crtc.cpp
	ioctrl.cpp
	kanji.cpp

	pc100.cpp
)

add_library(vm_emupc100
	${VM_emupc100_LIB_SRCS}
)
