cmake_minimum_required (VERSION 2.6)

message("* vm/j3100")

set(VM_J3100_LIB_SRCS
	display.cpp
	dmareg.cpp
	floppy.cpp
	j3100.cpp
	sasi.cpp
)

if(BUILD_J3100GT) 
  set(VM_J3100_LIB_SRCS ${VM_J3100_LIB_SRCS}
			  memory.cpp
			  system.cpp
			  keyboard.cpp
			  )
elseif(BUILD_J3100SL)
  set(VM_J3100_LIB_SRCS ${VM_J3100_LIB_SRCS}
			  slmemory.cpp
			  slsystem.cpp
			  slkeyboard.cpp
			  )
endif()


add_library(vm_j3100
	${VM_J3100_LIB_SRCS}
)