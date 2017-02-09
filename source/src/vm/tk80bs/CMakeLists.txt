cmake_minimum_required (VERSION 2.6)

message("* vm/tk80bs")

set(VMFILES_TK
	display.cpp
	keyboard.cpp

	membus.cpp
#	memory.cpp
)
if(BUILD_TK80BS)
   set(VMFILES_TK ${VMFILES_TK} cmt.cpp)
endif()
add_library(vm_tk80bs
	${VMFILES_TK}
	tk80bs.cpp
)