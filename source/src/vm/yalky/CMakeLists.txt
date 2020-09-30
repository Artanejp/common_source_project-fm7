cmake_minimum_required (VERSION 2.6)

message("* vm/yalky")

add_library(vm_emuyalky
	../i8080.cpp
	io.cpp
	yalky.cpp
)
