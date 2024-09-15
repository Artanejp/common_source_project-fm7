message("* vm/${EXE_NAME}")
set(BASIC_VM_FILES
	../sy6522.cpp
	
	./memory.cpp
	jr100.cpp
)
	   
add_library(vm_${EXE_NAME}  ${BASIC_VM_FILES})
