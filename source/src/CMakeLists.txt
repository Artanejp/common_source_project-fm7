message("* common/common")

#add_library(common_emu
#	emu.cpp
#)
if(USE_DEVICES_SHARED_LIB)
	add_library(common_${EXE_NAME}
		config.cpp
		debugger.cpp
	)
else()
	add_library(common_${EXE_NAME}
		common.cpp
		config.cpp
		config_dll.cpp
		debugger.cpp
		fileio.cpp
		fifo.cpp
		ringbuffer.cpp
	)
endif()

set_std(common_${EXE_NAME})
