cmake_minimum_required (VERSION 2.6)

message("* vm/pc9801")

set(BASIC_VM_FILES 
       pc9801.cpp
	   dipsw.cpp
	   display.cpp
	   floppy.cpp
	   fmsound.cpp
	   joystick.cpp
	   mouse.cpp
	   keyboard.cpp
	   dmareg.cpp
	   membus.cpp
	   )
	   
if(BUILD_PC9801) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} cmt.cpp )
elseif(BUILD_PC9801E) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} cmt.cpp )
elseif(BUILD_PC9801U) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} cmt.cpp )
elseif(BUILD_PC9801VF) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} )
elseif(BUILD_PC9801VM) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} )
#  add_library(vm_pc9801  ${BASIC_VM_FILES})
elseif(BUILD_PC98DO)
    add_library(vm_pc9801  ${BASIC_VM_FILES})
elseif(BUILD_PC9801VX) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} cpureg.cpp sasi.cpp sasi_bios.cpp egc.cpp)
elseif(BUILD_PC98XL) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} cpureg.cpp  sasi.cpp sasi_bios.cpp egc.cpp)
elseif(BUILD_PC9801RA) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} cpureg.cpp  sasi.cpp sasi_bios.cpp egc.cpp)
elseif(BUILD_PC98RL) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} cpureg.cpp sasi.cpp sasi_bios.cpp egc.cpp)
elseif(BUILD_PC98XA) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} cpureg.cpp sasi.cpp sasi_bios.cpp)
elseif(BUILD_PC98DOP) 
  add_library(vm_pc9801  ${BASIC_VM_FILES} sasi.cpp sasi_bios.cpp egc.cpp)
endif()
