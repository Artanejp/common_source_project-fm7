cmake_minimum_required (VERSION 2.6)

message("* vm/tk80bs")

add_library(vm_tk80bs
	display.cpp
	keyboard.cpp
	cmt.cpp
	tk80bs.cpp
)