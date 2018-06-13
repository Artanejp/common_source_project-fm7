cmake_minimum_required (VERSION 2.6)

message("* vm/fm16beta")

set(VM_FM16BETA_LIB_SRCS
     cmos.cpp
     keyboard.cpp
     mainbus.cpp
     sub.cpp
     fm16beta.cpp
)

if(USE_DEVICES_SHARED_LIB)
else()
	set(VM_FM16BETA_LIB_SRCS ${VM_FM16BETA_LIB_SRCS} 
	                         hd46505.cpp
	                         i8237.cpp
				 i8251.cpp
				 i8259.cpp
				 mb8877.cpp
				 mc6809.cpp
				 mc6840.cpp
				 msm58321.cpp
				 pcm1bit.cpp
				 
				 disk.cpp
				 
				 memory.cpp
				 io.cpp
				 )
endif()
add_library(vm_fm16beta
	${VM_FM16BETA_LIB_SRCS}
)
