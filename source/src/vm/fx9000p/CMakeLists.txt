cmake_minimum_required (VERSION 3.0)

message("* vm/${EXE_NAME}")

add_library(vm_emufx9000p
	./io.cpp
	./fx9000p.cpp
)


