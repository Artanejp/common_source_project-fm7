message("* vm/emufp1100")

add_library(vm_emufp1100
	../upd7801.cpp
	
	fdcpack.cpp
	fp1100.cpp
	main.cpp
	rampack.cpp
	rompack.cpp
	sub.cpp

)
