cmake_minimum_required (VERSION 2.6)

message("* vm/mz700")

if(BUILD_MZ800)

add_library(vm_mz700
	cmos.cpp
	emm.cpp
	kanji.cpp
	keyboard.cpp
	memory.cpp
	mz700.cpp
	ramfile.cpp
	floppy.cpp
	quickdisk.cpp
)

elseif(BUILD_MZ1500)

add_library(vm_mz700
	cmos.cpp
	emm.cpp
	kanji.cpp
	keyboard.cpp
	memory.cpp
	mz700.cpp
	ramfile.cpp
	floppy.cpp
	quickdisk.cpp
	ramfile.cpp
	psg.cpp
)

else()

add_library(vm_mz700
	cmos.cpp
	emm.cpp
	kanji.cpp
	keyboard.cpp
	memory.cpp
	mz700.cpp
	ramfile.cpp
)


endif()
