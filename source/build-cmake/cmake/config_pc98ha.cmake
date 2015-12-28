
set(VM_NAME pc98ha)
set(USE_FMGEN OFF)
set(VMFILES
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   
		   upd765a.cpp
		   upd71071.cpp
		   
		   beep.cpp
		   ls244.cpp
		   
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
		   prnfile.cpp
)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_PC98HA OFF CACHE BOOL "Build on PC98 HA")
set(BUILD_PC98LT OFF CACHE BOOL "Build on PC98 LT")
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")



include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_PC98HA)
   add_definitions(-D_PC98HA)
   set(EXEC_TARGET emupc98ha)
   set(VMFILES ${VMFILES}
		   upd4991a.cpp
		   i286.cpp
   )
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98ha.qrc)
elseif(BUILD_PC98LT)
   add_definitions(-D_PC98LT)
   set(EXEC_TARGET emupc98lt)
   set(VMFILES ${VMFILES}
		   upd1990a.cpp
		   i286.cpp
   )
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98lt.qrc)
endif()

include(config_commonsource)
