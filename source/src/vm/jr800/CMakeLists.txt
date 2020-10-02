cmake_minimum_required (VERSION 2.6)

message("* vm/emujr800")

add_library(vm_emujr800

	./io.cpp
	jr800.cpp
)
