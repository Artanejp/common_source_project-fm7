set(VM_NAME qc10)
set(USE_FMGEN OFF)
set(WITH_JOYSTICK OFF)
set(WITH_MOUSE OFF)

set(FLAG_USE_Z80 ON)
set(VMFILES
		   i8237.cpp
		   event.cpp
)
set(VMFILES_LIB
		   disk.cpp
		   noise.cpp
		   i8237_base.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   io.cpp
		   hd146818p.cpp
		   pcm1bit.cpp
		   upd7220.cpp
		   upd765a.cpp
		   z80sio.cpp
)		   

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(BUILD_QC10 OFF CACHE BOOL "Build emuqc10 (Monochrome)")
set(BUILD_QC10COLOR OFF CACHE BOOL "Build emuqc10_cms")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

add_definitions(-D_CONFIGURE_WITH_CMAKE)
add_definitions(-D_QC10)
if(BUILD_QC10COLOR)
  set(EXEC_TARGET emuqc10_cms)
  add_definitions(-D_COLOR_MONITOR)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/qc10cms.qrc)
else()
  set(EXEC_TARGET emuqc10)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/qc10.qrc)
endif()

include(config_commonsource)
