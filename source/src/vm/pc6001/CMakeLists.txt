cmake_minimum_required (VERSION 2.6)

message("* vm/pc6001")

set(BASIC_VM_FILES
#           display.cpp

	   joystick.cpp
	   memory.cpp
#	   memory_draw.cpp
	   pc6001.cpp
	   psub.cpp
	   sub.cpp
	   timer.cpp
)

if(BUILD_PC6601)
add_library(vm_pc6001
	   ${BASIC_VM_FILES}
	   memory_draw.cpp
	   floppy.cpp
	   )
elseif(BUILD_PC6601SR)
add_library(vm_pc6001
	   ${BASIC_VM_FILES}
	   memory_draw.cpp
	   floppy.cpp
	   )
elseif(BUILD_PC6001)
add_library(vm_pc6001
	   ${BASIC_VM_FILES}
	   display.cpp
	   )
else()
add_library(vm_pc6001
	   ${BASIC_VM_FILES}
	   memory_draw.cpp
	   )
endif()