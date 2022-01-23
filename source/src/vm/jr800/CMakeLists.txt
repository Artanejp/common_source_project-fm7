cmake_minimum_required (VERSION 3.0)

message("* vm/emujr800")

add_library(vm_emujr800

	./io.cpp
	jr800.cpp
)
