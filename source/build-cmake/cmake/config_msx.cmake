# Build Common Sourcecode Project, Qt.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of , but license is apache 2.2,
# this part was written only me.

message("")
message("** Start of configure CommonSourceProject,MSX Series Qt **")
message("")

set(VMFILES_BASE
	event.cpp
	io.cpp
	memory.cpp
)

set(VMFILES_LIB
	datarec.cpp
	ay_3_891x.cpp
	i8255.cpp
	not.cpp
	pcm1bit.cpp
	ym2413.cpp
	prnfile.cpp
)
set(FLAG_USE_Z80 ON)

#set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)
add_definitions(-D_MSX_VDP_MESS)

set(VMFILES_MSX2 ${VMFILES_BASE}
)

set(VMFILES_MSX2_LIB ${VMFILES_LIB}
	v9938.cpp
	rp5c01.cpp
)

set(VMFILES_LIB ${VMFILES_LIB}	disk.cpp)

set(VMFILES_MSX1 ${VMFILES_BASE}	
)
set(VMFILES_LIB_MSX1 ${VMFILES_LIB}	
	tms9918a.cpp
	disk.cpp
)


set(VMFILES_PX7 ${VMFILES_BASE}
	ld700.cpp
)
set(VMFILES_LIB_PX7 ${VMFILES_LIB}
	tms9918a.cpp
)

set(VMFILES_HX20 ${VMFILES_BASE}	
)

set(VMFILES_LIB_HX20 ${VMFILES_LIB}	
	tms9918a.cpp
	disk.cpp
)

set(VMFILES_FSA1 ${VMFILES_BASE}
)
set(VMFILES_LIB_FSA1 ${VMFILES_LIB}
	rp5c01.cpp
	v9938.cpp
	disk.cpp
)

set(VMFILES_HBF1XDJ ${VMFILES_BASE}
)
set(VMFILES_LIB_HBF1XDJ ${VMFILES_LIB}
	rp5c01.cpp
	v9938.cpp
	disk.cpp
)

set(VMFILES_MSX2PLUS ${VMFILES_BASE}
)

set(VMFILES_LIB_MSX2PLUS ${VMFILES_LIB}
	rp5c01.cpp
	v9938.cpp
	disk.cpp
)

if(BUILD_PX7)
  set(VMFILES ${VMFILES_PX7})
  set(VMFILES_LIB ${VMFILES_LIB_PX7})
  add_definitions(-D_PX7)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/px7.qrc)
elseif(BUILD_MSX2)
  set(VMFILES ${VMFILES_MSX2})
  set(VMFILES_LIB ${VMFILES_LIB_MSX2})
  add_definitions(-D_MSX2)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/msx2.qrc)
elseif(BUILD_MSX2PLUS)
  set(VMFILES ${VMFILES_MSX2PLUS})
  set(VMFILES_LIB ${VMFILES_LIB_MSX2PLUS})
  add_definitions(-D_MSX2P)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/msx2plus.qrc)
elseif(BUILD_HX20)
  set(VMFILES ${VMFILES_HX20})
  set(VMFILES_LIB ${VMFILES_LIB_HX20})
  add_definitions(-D_HX20)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/hx20.qrc)
elseif(BUILD_FSA1)
  set(VMFILES ${VMFILES_FSA1})
  set(VMFILES_LIB ${VMFILES_LIB_FSA1})
  add_definitions(-D_FSA1)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/fsa1.qrc)
elseif(BUILD_HBF1XDJ)
  set(VMFILES ${VMFILES_HBF1XDJ})
  set(VMFILES_LIB ${VMFILES_LIB_HBF1XDJ})
  add_definitions(-D_HBF1XDJ)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/hbf1xdj.qrc)
else()
  set(VMFILES ${VMFILES_MSX1})
  set(VMFILES_LIB ${VMFILES_LIB_MSX1})
  add_definitions(-D_MSX1)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/msx1.qrc)
endif()

if(USE_CMT_SOUND)
       set(VMFILES_MSX ${VMFILES_BASE})
endif()

include(config_commonsource)


                         


