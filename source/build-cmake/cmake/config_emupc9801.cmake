set(LOCAL_LIBS 	   vm_pc98
		   vm_vm
		   common_common
		   vm_fmgen
#		   common_scaler-generic
                   qt_pc9801
		   qt_gui
                  )

set(VMFILES
		   i8237.cpp
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   
		   pc80s31k.cpp
		   upd1990a.cpp
		   upd765a.cpp
		   upd7220.cpp
		   tms3631.cpp
		   
		   ym2203.cpp
		   ls244.cpp
		   
		   disk.cpp
		   event.cpp
		   io.cpp
		   memory.cpp
)
set(BUILD_SHARED_LIBS OFF)

set(BUILD_PC9801 OFF CACHE BOOL "Build on PC9801")
set(BUILD_PC9801E OFF CACHE BOOL "Build on PC9801E")
set(BUILD_PC9801U OFF CACHE BOOL "Build on PC9801U")
set(BUILD_PC9801VF OFF CACHE BOOL "Build on PC9801VF")
set(BUILD_PC9801VM OFF CACHE BOOL "Build on PC9801VM")
set(BUILD_PC98DO OFF CACHE BOOL "Build on PC98DO")
set(USE_CMT_SOUND ON CACHE BOOL "Sound with Data Recorder.")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_PC9801)
   add_definitions(-D_PC9801)
   set(EXEC_TARGET emupc9801)
   set(VMFILES ${VMFILES}
       beep.cpp
       i286.cpp
       z80.cpp
       not.cpp
   )
   if(USE_CMT_SOUND)
       add_definitions(-DDATAREC_SOUND)
   endif()
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801.qrc)
elseif(BUILD_PC9801E)
   add_definitions(-D_PC9801E)
   set(EXEC_TARGET emupc9801e)
   set(VMFILES ${VMFILES}
       beep.cpp
       not.cpp
       i286.cpp
       z80.cpp
   )
   if(USE_CMT_SOUND)
       add_definitions(-DDATAREC_SOUND)
   endif()
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801e.qrc)
elseif(BUILD_PC9801U)
   add_definitions(-D_PC9801U)
   set(EXEC_TARGET emupc9801u)
   set(VMFILES ${VMFILES}
       beep.cpp
       i286.cpp
       z80.cpp
       not.cpp
       pcm1bit.cpp
   )
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801u.qrc)
elseif(BUILD_PC9801VM)
   add_definitions(-D_PC9801VM)
   set(EXEC_TARGET emupc9801vm)
   set(VMFILES ${VMFILES}
       pcm1bit.cpp
       i286.cpp
       not.cpp
       )
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801vm.qrc)
elseif(BUILD_PC9801VF)
   add_definitions(-D_PC9801VF)
   set(EXEC_TARGET emupc9801vf)
   set(VMFILES ${VMFILES}
       pcm1bit.cpp
       i286.cpp
       not.cpp
       )
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801vf.qrc)
elseif(BUILD_PC98DO)
   add_definitions(-D_PC98DO)
   set(EXEC_TARGET emupc98do)
   set(VMFILES ${VMFILES}
       beep.cpp
       pcm1bit.cpp
       
       i286.cpp
       upd4991a.cpp
       
       pc80s31k.cpp
       z80.cpp
       
       not.cpp
   )
   set(LOCAL_LIBS ${LOCAL_LIBS}
       vm_pc88
   )
   if(USE_CMT_SOUND)
       add_definitions(-DDATAREC_SOUND)
   endif()
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98do.qrc)
endif()


#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc9801)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fmgen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/pc9801)
if(BUILD_PC98DO)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/pc8801)
endif()

include(config_commonsource)

if(USE_SSE2)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/fm7/vram/sse2)
#  add_subdirectory(../../src/vm/fm7/vram/sse2 vm/fm7/vram/sse2)
endif()


if(USE_SSE2)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/agar/common/scaler/sse2)
endif()

