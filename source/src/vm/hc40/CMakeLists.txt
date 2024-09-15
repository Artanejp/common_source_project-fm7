message("* vm/${EXE_NAME}")

add_library(vm_${EXE_NAME}
	./io.cpp
	./memory.cpp
	hc40.cpp
)
