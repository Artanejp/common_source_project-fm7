cmake_minimum_required (VERSION 2.6)

message("* vm/smb80te")

add_library(vm_emusmb80te
	smb80te.cpp
	memory.cpp
)
