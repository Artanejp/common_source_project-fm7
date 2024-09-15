message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
	./memory.cpp
	
	./hc20.cpp
)
