cmake_minimum_required (VERSION 2.6)

message("* vm/mz700")

add_library(vm_mz700
	cmos.cpp
	emm.cpp
	kanji.cpp
	keyboard.cpp
	memory.cpp
	mz700.cpp
	ramfile.cpp
)

if(BUILD_MZ800)
add_library(vm_mz700
	floppy.cpp
	quickdisk.cpp
)
endif()

if(BUILD_MZ1500)
if(not BUILD_MZ800)
add_library(vm_mz700
	floppy.cpp
	quickdisk.cpp
	ramfile.cpp
	psg.cpp
)
endif()
endif()
