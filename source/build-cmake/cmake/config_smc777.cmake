
set(VM_NAME smc777)
set(USE_FMGEN OFF)
set(WITH_MOUSE ON)
set(WITH_JOYSTICK ON)

set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(BUILD_SMC70 OFF CACHE BOOL "Build SMC-70")
set(BUILD_SMC777 OFF CACHE BOOL "Build SMC-777")
set(WITH_DEBUGGER ON CACHE BOOL "Build with debugger")

include(detect_target_cpu)
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

set(FLAG_USE_Z80 ON)
if(BUILD_SMC70)
  set(EXEC_TARGET emusmc70)
  set(VMFILES_BASE
  	   msm58321.cpp
	   event.cpp
  )
set(VMFILES_LIB
	   noise.cpp
	   datarec.cpp
	   hd46505.cpp
	   mb8877.cpp
	   msm58321_base.cpp
	   pcm1bit.cpp
	   disk.cpp
)
 add_definitions(-D_SMC70)
 set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/smc70.qrc)
elseif(BUILD_SMC777)
  
  set(EXEC_TARGET emusmc777)
  set(VMFILES_BASE
	event.cpp
  )
  set(VMFILES_LIB
	datarec.cpp
	hd46505.cpp
	sn76489an.cpp	
	pcm1bit.cpp
	mb8877.cpp
	disk.cpp
  )
  add_definitions(-D_SMC777)
  set(RESOURCE ${CMAKE_SOURCE_DIR}/../../src/qt/common/qrc/smc777.qrc)
endif()

set(VMFILES ${VMFILES_BASE})

include(config_commonsource)

