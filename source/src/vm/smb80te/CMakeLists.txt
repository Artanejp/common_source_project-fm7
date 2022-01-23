cmake_minimum_required (VERSION 3.0)

message("* vm/smb80te")

add_library(vm_emusmb80te
	smb80te.cpp
	memory.cpp
)
