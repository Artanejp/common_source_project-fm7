message("* vm/${EXE_NAME}")

string(TOUPPER "${EXE_NAME}" U_EXE_NAME)

add_library(vm_${EXE_NAME}
		./memory.cpp
		./keyboard.cpp

		./micom_mahjong.cpp
	)
