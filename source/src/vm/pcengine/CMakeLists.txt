cmake_minimum_required (VERSION 3.0)

message("* vm/emupcengine")

add_library(vm_emupcengine
	../scsi_host.cpp
	../huc6280.cpp
	
	pce.cpp
	adpcm.cpp	   
	pcengine.cpp
)

