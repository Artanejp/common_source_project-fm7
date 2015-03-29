cmake_minimum_required (VERSION 2.6)

message("* vm/fp1100")

add_library(vm_fp1100
	fdcpack.cpp
	fp1100.cpp
	main.cpp
	rampack.cpp
	rompack.cpp
	sub.cpp

)