cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0011 NEW)

set(VM_NAME pc8801)
set(USE_FMGEN ON)
set(USE_DEBUGGER ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(VMFILES
		event.cpp
		io.cpp
		memory.cpp
)
set(VMFILES_LIB
		noise.cpp
		beep.cpp
		datarec.cpp
		i8251.cpp
		i8255.cpp
  
		pc80s31k.cpp
		pcm1bit.cpp
		upd1990a.cpp
		upd765a.cpp
		ym2203.cpp
		z80ctc.cpp
		z80dma.cpp
		z80pio.cpp
		z80sio.cpp
		disk.cpp
		
		prnfile.cpp
)
set(FLAG_USE_Z80 ON)

set(BUILD_SHARED_LIBS OFF)

set(BUILD_PC8001SR OFF CACHE BOOL "Build for PC8001SR")
set(BUILD_PC8801MA OFF CACHE BOOL "Build with PC8801MA")
set(USE_OPNA ON CACHE BOOL "Use OPNA sound with PC8801MA")
set(USE_SOUNDBOARD2 ON CACHE BOOL "Use Sound Board sound with PC8801MA")
set(USE_PCG  ON CACHE BOOL "Use PCG8100")
set(PC88_EXTRAM_PAGES  "4" CACHE STRING "Set banks of EXTRAM of PC8801, bank = 32Kbytes")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with Debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)



if(BUILD_PC8001SR)
  set(EXEC_TARGET emupc8001sr)
  add_definitions(-D_PC8001SR)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc8001mk2sr.qrc)
  
elseif(BUILD_PC8801MA)
  set(EXEC_TARGET emupc8801ma)
  add_definitions(-D_PC8801MA)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc8801ma.qrc)
  set(VMFILES_LIB ${VMFILES_LIB}
            ym2151.cpp
  )      
  set(VMFILES ${VMFILES}
            scsi_dev.cpp scsi_cdrom.cpp scsi_host.cpp
  )      
endif()

add_definitions(-DPC88_EXRAM_BANKS=${PC88_EXTRAM_PAGES})

if(USE_PCG)
  set(VMFILES_LIB ${VMFILES_LIB}
            i8253.cpp
      )      
  add_definitions(-DSUPPORT_PC88_PCG8100)
endif()

if(USE_SOUNDBOARD2)
  add_definitions(-DSUPPORT_PC88_SB2)
  add_definitions(-DSUPPORT_PC88_OPNA)
else()  
 if(USE_OPNA)
    add_definitions(-DSUPPORT_PC88_OPNA)
 endif()
endif() 

include(config_commonsource)
