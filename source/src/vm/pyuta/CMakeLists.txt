cmake_minimum_required (VERSION 2.6)

message("* vm/pyuta")

  add_library(vm_pyuta
	memory.cpp
	pyuta.cpp
  )
