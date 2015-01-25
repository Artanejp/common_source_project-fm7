cmake_minimum_required (VERSION 2.6)

message("* vm/pc9801")

if(BUILD_PC9801VM) 
  add_library(vm_pc98
           pc9801.cpp
	   display.cpp
	   floppy.cpp
	   fmsound.cpp
	   joystick.cpp
	   keyboard.cpp
	   mouse.cpp
	   printer.cpp
  )
endif()