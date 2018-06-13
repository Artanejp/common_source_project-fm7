
set(VM_NAME pc9801)
set(USE_FMGEN ON)
set(WITH_JOYSTICK ON)
set(WITH_MOUSE ON)

set(VMFILES
		   i8237.cpp
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
		   tms3631.cpp
		   upd1990a.cpp
		   upd7220_base.cpp
		   upd765a.cpp
		   ym2203.cpp
		   prnfile.cpp
		   noise.cpp
		   disk.cpp
)

set(BUILD_SHARED_LIBS OFF)

set(BUILD_PC9801 OFF CACHE BOOL "Build on PC9801")
set(BUILD_PC9801E OFF CACHE BOOL "Build on PC9801E")
set(BUILD_PC9801RA OFF CACHE BOOL "Build on PC9801RA")
set(BUILD_PC9801U OFF CACHE BOOL "Build on PC9801U")
set(BUILD_PC9801VF OFF CACHE BOOL "Build on PC9801VF")
set(BUILD_PC9801VM OFF CACHE BOOL "Build on PC9801VM")
set(BUILD_PC9801VX OFF CACHE BOOL "Build on PC9801VX")
set(BUILD_PC98DO OFF CACHE BOOL "Build on PC98DO")
set(BUILD_PC98DOP OFF CACHE BOOL "Build on PC98DO+")
set(BUILD_PC98RL OFF CACHE BOOL "Build on PC9801RL/XL^2")
set(BUILD_PC98XA OFF CACHE BOOL "Build on PC98XA")
set(BUILD_PC98XL OFF CACHE BOOL "Build on PC98XL")

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
   )
   set(VMFILES_LIB 
       beep.cpp
       not.cpp
	   ${VMFILES_LIB}
	   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#   set(FLAG_USE_I86 ON)
   set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801.qrc)
elseif(BUILD_PC9801E)
   add_definitions(-D_PC9801E)
   set(EXEC_TARGET emupc9801e)
   set(VMFILES ${VMFILES}
   )
   set(FLAG_USE_Z80 ON)
   set(VMFILES_LIB 
       beep.cpp
       not.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#  set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801e.qrc)
elseif(BUILD_PC9801U)
   add_definitions(-D_PC9801U)
   set(EXEC_TARGET emupc9801u)
   set(VMFILES ${VMFILES}
   )
   set(FLAG_USE_Z80 ON)
   set(VMFILES_LIB 
       beep.cpp
       not.cpp
       pcm1bit.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#    set(FLAG_USE_I86 ON)
 set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801u.qrc)
elseif(BUILD_PC9801VM)
   add_definitions(-D_PC9801VM)
   set(EXEC_TARGET emupc9801vm)
   set(VMFILES ${VMFILES}
       )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#   set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801vm.qrc)
elseif(BUILD_PC9801VX)
   add_definitions(-D_PC9801VX)
   set(EXEC_TARGET emupc9801vx)
   set(VMFILES ${VMFILES}
       scsi_host.cpp
       scsi_dev.cpp
       scsi_hdd.cpp
   )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#   set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801vx.qrc)
elseif(BUILD_PC98XA)
   add_definitions(-D_PC98XA)
   set(EXEC_TARGET emupc98xa)
   set(VMFILES ${VMFILES}
       scsi_host.cpp
	   scsi_dev.cpp
	   scsi_hdd.cpp
       )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#   set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98xa.qrc)
elseif(BUILD_PC98XL)
   add_definitions(-D_PC98XL)
   set(EXEC_TARGET emupc98xl)
   set(VMFILES ${VMFILES}
       scsi_host.cpp
	   scsi_dev.cpp
	   scsi_hdd.cpp
       )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#   set(FLAG_USE_I86 ON)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98xl.qrc)
elseif(BUILD_PC9801VF)
   add_definitions(-D_PC9801VF)
   set(EXEC_TARGET emupc9801vf)
   set(VMFILES ${VMFILES}
       )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#    set(FLAG_USE_I86 ON)
 set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801vf.qrc)
elseif(BUILD_PC9801RA)
   add_definitions(-D_PC9801RA)
   set(EXEC_TARGET emupc9801ra)
   set(VMFILES ${VMFILES}
       scsi_host.cpp
	   scsi_dev.cpp
	   scsi_hdd.cpp
       )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS ON)
  set(FLAG_USE_I286 OFF)
#    set(FLAG_USE_I86 ON)
 set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc9801ra.qrc)
elseif(BUILD_PC98RL)
   add_definitions(-D_PC98RL)
   set(EXEC_TARGET emupc98rl)
   set(VMFILES ${VMFILES}
       scsi_host.cpp
	   scsi_dev.cpp
	   scsi_hdd.cpp
       )
   set(VMFILES_LIB 
       not.cpp
       pcm1bit.cpp
	   ${VMFILES_LIB}
   )
  set(FLAG_USE_I386_VARIANTS ON)
  set(FLAG_USE_I286 OFF)
#    set(FLAG_USE_I86 ON)
 set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98rl.qrc)
elseif(BUILD_PC98DO)
   add_definitions(-D_PC98DO)
   set(EXEC_TARGET emupc98do)
   set(VMFILES ${VMFILES}
   )
   set(FLAG_USE_Z80 ON)
   set(VMFILES_LIB ${VMFILES_LIB}
       pc80s31k.cpp
       beep.cpp
       not.cpp
       pcm1bit.cpp
       upd4991a.cpp
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#   set(FLAG_USE_I86 ON)
   set(VM_APPEND_LIBS vm_pc8801)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98do.qrc)
elseif(BUILD_PC98DOP)
   add_definitions(-D_PC98DOPLUS)
   set(EXEC_TARGET emupc98doplus)
   set(VMFILES ${VMFILES}
       scsi_host.cpp
	   scsi_dev.cpp
	   scsi_hdd.cpp
   )
   set(FLAG_USE_Z80 ON)
   set(VMFILES_LIB ${VMFILES_LIB}
       pc80s31k.cpp
       beep.cpp
       not.cpp
       pcm1bit.cpp
       upd4991a.cpp
   )
  set(FLAG_USE_I386_VARIANTS OFF)
  set(FLAG_USE_I286 ON)
#   set(FLAG_USE_I86 ON)
   set(VM_APPEND_LIBS vm_pc8801)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/pc98doplus.qrc)
endif()


if(BUILD_PC98DO)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc8801)
elseif(BUILD_PC98DOP)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src/vm/pc8801)
endif()

include(config_commonsource)

if(BUILD_PC98DO)
	add_subdirectory(../../src/vm/pc8801 vm/pc8801)
endif()
