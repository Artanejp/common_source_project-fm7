cmake_minimum_required (VERSION 2.6)

message("* vm/pc9801")

set(BASIC_VM_FILES 
           pc9801.cpp
	   display.cpp
	   floppy.cpp
	   fmsound.cpp
	   joystick.cpp
	   mouse.cpp
	   printer.cpp
	   keyboard.cpp
	   )
	   
if(BUILD_PC9801) 
  add_library(vm_pc98  ${BASIC_VM_FILES} cmt.cpp )
elseif(BUILD_PC9801E) 
  add_library(vm_pc98  ${BASIC_VM_FILES} cmt.cpp )
elseif(BUILD_PC9801U) 
  add_library(vm_pc98  ${BASIC_VM_FILES} cmt.cpp )
elseif(BUILD_PC9801VF) 
  add_library(vm_pc98  ${BASIC_VM_FILES} )
elseif(BUILD_PC9801VM) 
  add_library(vm_pc98  ${BASIC_VM_FILES} )
elseif(BUILD_PC98DO) 
  add_library(vm_pc98  ${BASIC_VM_FILES})
endif()