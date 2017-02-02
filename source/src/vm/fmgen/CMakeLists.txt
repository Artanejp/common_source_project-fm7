cmake_minimum_required (VERSION 2.6)

message("* vm/fmgen")

add_library(vm_fmgen
#	             file.cpp
	             fmgen.cpp
		     fmtimer.cpp
		     opm.cpp
		     opna.cpp
		     psg.cpp
		     psg_ay_3_891x.cpp
)