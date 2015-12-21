cmake_minimum_required (VERSION 2.6)

message("* vm/fm7")


if(BUILD_GAMEGEAR)
add_library(vm_gamegear
	gamegear.cpp
	keyboard.cpp
	memory.cpp
	system.cpp
)
else()
add_library(vm_gamegear
	mastersystem.cpp
	keyboard.cpp
	memory.cpp
	system.cpp
)
endif()
