message("* vm/${EXE_NAME}")

add_library(vm_emupc2001
	../upd7907.cpp
	
	./io.cpp
	pc2001.cpp
)
