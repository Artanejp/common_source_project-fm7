cmake_minimum_required (VERSION 2.6)

message("* vm/msx1")

if(BUILD_PX7)
  add_library(vm_msx
	joystick.cpp
	keyboard.cpp
	memory.cpp
	msx.cpp
  )
elseif(BUILD_MSX2)
  add_library(vm_msx
	joystick.cpp
	keyboard.cpp
	memory.cpp
	rtcif.cpp
	msx.cpp
  )
else()
  add_library(vm_msx
	joystick.cpp
	keyboard.cpp
	memory.cpp
	msx.cpp
  )
endif()

