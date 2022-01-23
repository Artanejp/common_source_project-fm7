cmake_minimum_required (VERSION 3.0)

message("* vm/yalky")

add_library(vm_emuyalky
	../i8080.cpp
	io.cpp
	yalky.cpp
)
