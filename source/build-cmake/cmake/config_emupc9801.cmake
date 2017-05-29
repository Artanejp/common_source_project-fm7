
set(VM_NAME pc9801)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(VMFILES
		   i8237.cpp
		   i8253.cpp
		   upd7220.cpp

		   event.cpp
		   io.cpp
		   memory.cpp
)
set(VMFILES_LIB
		   i8237_base.cpp
		   i8251.cpp
		   i8253.cpp
		   i8255.cpp
		   i8259.cpp
		   ls244.cpp
		   pc80s31k.cpp
		   upd1990a.cpp
		   upd7220_base.cpp
		   upd765a.cpp
		   ym2203.cpp
		   prnfile.cpp
		   disk.cpp
)

set(BUILD_SHARED_LIBS OFF)

set(BUILD_PC9801 OFF CACHE BOOL "Build on PC9801")
set(BUILD_PC9801E OFF CACHE BOOL "Build on PC9801E")
set(BUILD_PC9801U OFF CACHE BOOL "Build on PC9801U")
set(BUILD_PC9801VF OFF CACHE BOOL "Build on PC9801VF")
set(BUILD_PC9801VM OFF CACHE BOOL "Build on PC9801VM")
set(BUILD_PC98DO OFF CACHE BOOL "Build on PC98DO")

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger.")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

if(BUILD_PC9801)
   add_definitions(-D_PC9801)
   set(EXEC_TARGET emupc9801)
   set(FLAG_USE_Z80 ON)
   set(VMFILES ${VMFILES}
       i286.cpp
   )
   set(VMFILES_LIB 
       beep.cpp
       not.cpp
	   )
#   set(FLAG_USE_I86 ON)
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801.qrc)
elseif(BUILD_PC9801E)
   add_definitions(-D_PC9801E)
   set(EXEC_TARGET emupc9801e)
   set(VMFILES ${VMFILES}
       i286.cpp
   )
   set(FLAG_USE_Z80 ON)
   set(VMFILES_LIB 
       beep.cpp
       not.cpp
   )
#  set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801e.qrc)
elseif(BUILD_PC9801U)
   add_definitions(-D_PC9801U)
   set(EXEC_TARGET emupc9801u)
   set(VMFILES ${VMFILES}
       i286.cpp
   )
   set(FLAG_USE_Z80 ON)
   set(VMFILES_LIB 
       beep.cpp
       not.cpp
       pcm1bit.cpp
   )
#    set(FLAG_USE_I86 ON)
 set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801u.qrc)
elseif(BUILD_PC9801VM)
   add_definitions(-D_PC9801VM)
   set(EXEC_TARGET emupc9801vm)
   set(VMFILES ${VMFILES}
       i286.cpp
       )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
   )
#   set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801vm.qrc)
elseif(BUILD_PC9801VF)
   add_definitions(-D_PC9801VF)
   set(EXEC_TARGET emupc9801vf)
   set(VMFILES ${VMFILES}
       i286.cpp
       )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
   )
#    set(FLAG_USE_I86 ON)
 set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801vf.qrc)
elseif(BUILD_PC98DO)
   add_definitions(-D_PC98DO)
   set(EXEC_TARGET emupc98do)
   set(VMFILES ${VMFILES}
       i286.cpp
       
   )
   set(FLAG_USE_Z80 ON)
   set(VMFILES_LIB ${VMFILES_LIB}
       pc80s31k.cpp
       beep.cpp
       not.cpp
       pcm1bit.cpp
       upd4991a.cpp
   )
#   set(FLAG_USE_I86 ON)
   set(VM_APPEND_LIBS vm_pc8801)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98do.qrc)
endif()


if(BUILD_PC98DO)
#  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/qt/pc8801)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc8801)
endif()

include(config_commonsource)

if(BUILD_PC98DO)
	add_subdirectory(../../src/vm/pc8801 vm/pc8801)
endif()
